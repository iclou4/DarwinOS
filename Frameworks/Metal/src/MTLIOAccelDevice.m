/*
 * MTLIOAccelDevice.m - ravynOS Metal: IOKit Hardware Accelerator device
 */

#import <Metal/MTLIOAccelDevice.h>
#import <Metal/MTLIOAccelCommandQueue.h>
#import <Metal/MTLIOAccelBuffer.h>
#import <Metal/MTLIOAccelTexture.h>
#import "MTLSoftwareSampler.h"
#import <IOKit/IOKitLib.h>
#import <IOKit/graphics/IOGraphicsLib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <dlfcn.h>

#ifndef kIOMasterPortDefault
#define kIOMasterPortDefault MACH_PORT_NULL
#endif

typedef io_service_t (*IOServiceGetMatchingService_fn)(mach_port_t, CFDictionaryRef);
typedef CFMutableDictionaryRef (*IOServiceMatching_fn)(const char *);
typedef kern_return_t (*IOServiceOpen_fn)(io_service_t, task_port_t, uint32_t, io_connect_t *);
typedef kern_return_t (*IOServiceClose_fn)(io_connect_t);
typedef kern_return_t (*IOObjectRelease_fn)(io_object_t);

static IOServiceGetMatchingService_fn s_IOServiceGetMatchingService = NULL;
static IOServiceMatching_fn s_IOServiceMatching = NULL;
static IOServiceOpen_fn s_IOServiceOpen = NULL;
static IOServiceClose_fn s_IOServiceClose = NULL;
static IOObjectRelease_fn s_IOObjectRelease = NULL;

static void InitIOKitSymbols(void)
{
	static dispatch_once_t onceToken;
	dispatch_once(&onceToken, ^{
		s_IOServiceGetMatchingService = (IOServiceGetMatchingService_fn)dlsym(RTLD_DEFAULT, "IOServiceGetMatchingService");
		s_IOServiceMatching = (IOServiceMatching_fn)dlsym(RTLD_DEFAULT, "IOServiceMatching");
		s_IOServiceOpen = (IOServiceOpen_fn)dlsym(RTLD_DEFAULT, "IOServiceOpen");
		s_IOServiceClose = (IOServiceClose_fn)dlsym(RTLD_DEFAULT, "IOServiceClose");
		s_IOObjectRelease = (IOObjectRelease_fn)dlsym(RTLD_DEFAULT, "IOObjectRelease");
	});
}

@implementation MTLIOAccelDevice

@synthesize acceleratorPort = _acceleratorPort;
@synthesize sharedMemorySize = _sharedMemorySize;
@synthesize dedicatedMemorySize = _dedicatedMemorySize;
@synthesize isHardwareActive = _isHardwareActive;

+ (instancetype)defaultDevice
{
	static MTLIOAccelDevice *s_accelDevice = nil;
	static dispatch_once_t s_once;
	dispatch_once(&s_once, ^{
		InitIOKitSymbols();
		io_service_t accel = IO_OBJECT_NULL;

		if (s_IOServiceMatching && s_IOServiceGetMatchingService) {
			// 1. Probe for IOKit IOAccelerator service
			CFDictionaryRef matchAccel = s_IOServiceMatching("IOAccelerator");
			if (matchAccel) {
				accel = s_IOServiceGetMatchingService(MACH_PORT_NULL, matchAccel);
			}

			// 2. Fall back to IOFramebuffer / IOGOPFramebuffer service
			if (accel == IO_OBJECT_NULL) {
				CFDictionaryRef matchFB = s_IOServiceMatching("IOFramebuffer");
				if (matchFB) {
					accel = s_IOServiceGetMatchingService(MACH_PORT_NULL, matchFB);
				}
			}
		}

		// Instantiate hardware accelerator device
		s_accelDevice = [[MTLIOAccelDevice alloc] initWithAcceleratorPort:accel];
	});
	return s_accelDevice;
}

- (instancetype)initWithAcceleratorPort:(io_service_t)port
{
	self = [super init];
	if (self) {
		InitIOKitSymbols();
		_acceleratorPort = port;
		_acceleratorConnect = IO_OBJECT_NULL;
		_isHardwareActive = YES;
		_sharedMemorySize = 1024ULL * 1024ULL * 1024ULL; // 1GB
		_dedicatedMemorySize = 256ULL * 1024ULL * 1024ULL; // 256MB VRAM
		_name = @"Apple GPU (IOAccelerator / Paravirtualized Graphics)";
		_registryID = 0x10002;

		// Open IOKit connection if service is valid
		if (port != IO_OBJECT_NULL && s_IOServiceOpen) {
			s_IOServiceOpen(port, mach_task_self(), 0, &_acceleratorConnect);
		}

		// Check for framebuffer aperture (via /dev/fb0 or direct video console)
		_framebufferWidth = 1280;
		_framebufferHeight = 800;
		_framebufferPitch = 1280 * 4;
		_framebufferAperture = NULL;

		int fd = open("/dev/fb0", O_RDWR);
		if (fd >= 0) {
			size_t mapSize = _framebufferPitch * _framebufferHeight;
			void *map = mmap(NULL, mapSize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
			if (map != MAP_FAILED) {
				_framebufferAperture = map;
			}
			close(fd);
		}
	}
	return self;
}

- (void)dealloc
{
	if (_framebufferAperture) {
		munmap(_framebufferAperture, _framebufferPitch * _framebufferHeight);
		_framebufferAperture = NULL;
	}
	if (_acceleratorConnect != IO_OBJECT_NULL && s_IOServiceClose) {
		s_IOServiceClose(_acceleratorConnect);
		_acceleratorConnect = IO_OBJECT_NULL;
	}
	if (_acceleratorPort != IO_OBJECT_NULL && s_IOObjectRelease) {
		s_IOObjectRelease(_acceleratorPort);
		_acceleratorPort = IO_OBJECT_NULL;
	}
}

- (id<MTLCommandQueue>)newCommandQueue
{
	return [self newCommandQueueWithMaxCommandBufferCount:64];
}

- (id<MTLCommandQueue>)newCommandQueueWithMaxCommandBufferCount:(NSUInteger)maxCommandBufferCount
{
	return [[MTLIOAccelCommandQueue alloc] initWithDevice:self maxCommandBufferCount:maxCommandBufferCount];
}

- (id<MTLBuffer>)newBufferWithLength:(NSUInteger)length options:(MTLResourceOptions)options
{
	return [[MTLIOAccelBuffer alloc] initWithDevice:self length:length options:options];
}

- (id<MTLBuffer>)newBufferWithBytes:(const void *)pointer length:(NSUInteger)length options:(MTLResourceOptions)options
{
	return [[MTLIOAccelBuffer alloc] initWithDevice:self bytes:pointer length:length options:options];
}

- (id<MTLTexture>)newTextureWithDescriptor:(MTLTextureDescriptor *)descriptor
{
	return [[MTLIOAccelTexture alloc] initWithDevice:self descriptor:descriptor];
}

- (id<MTLSamplerState>)newSamplerStateWithDescriptor:(MTLSamplerDescriptor *)descriptor
{
	return [[MTLSoftwareSamplerState alloc] initWithDevice:self descriptor:descriptor];
}

- (void)presentPixels:(const void *)pixels width:(NSUInteger)width height:(NSUInteger)height pitch:(NSUInteger)pitch
{
	if (!pixels) return;

	// 1. If direct mapped framebuffer aperture is available, copy to VRAM
	if (_framebufferAperture) {
		uint8_t *dst = (uint8_t *)_framebufferAperture;
		const uint8_t *src = (const uint8_t *)pixels;
		NSUInteger copyWidth = (width < _framebufferWidth ? width : _framebufferWidth) * 4;
		NSUInteger copyHeight = height < _framebufferHeight ? height : _framebufferHeight;
		for (NSUInteger y = 0; y < copyHeight; y++) {
			memcpy(dst + y * _framebufferPitch, src + y * pitch, copyWidth);
		}
		return;
	}

	// 2. Direct IOKit / Darwin hardware scanout syscall
#if defined(__APPLE__) || defined(__ravynOS__) || defined(__Darwin__)
	register long rax __asm__("rax") = (0x2000000 | 535); // SYS_FB_PRESENT
	register long rdi __asm__("rdi") = (long)pixels;
	register long rsi __asm__("rsi") = (long)width;
	register long rdx __asm__("rdx") = (long)height;
	register long r10 __asm__("r10") = (long)pitch;
	__asm__ volatile("syscall"
	    : "+r"(rax)
	    : "r"(rdi), "r"(rsi), "r"(rdx), "r"(r10)
	    : "rcx", "r11", "memory");
#endif
}

@end

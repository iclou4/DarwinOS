/*
 * MTLSoftwareDevice.m - ravynOS Metal: software device implementation
 */

#import "MTLSoftwareDevice.h"
#import "MTLSoftwareCommandQueue.h"
#import "MTLSoftwareBuffer.h"
#import "MTLSoftwareTexture.h"
#import "MTLSoftwareSampler.h"
#import "MTLSoftwareLibrary.h"
#import "MTLSoftwareRenderPipeline.h"
#import "MTLSoftwareDepthStencil.h"

@implementation MTLSoftwareDevice

@synthesize cDevice = _cDevice;

+ (instancetype)sharedDevice
{
	static MTLSoftwareDevice *sDevice = nil;
	static dispatch_once_t onceToken;
	dispatch_once(&onceToken, ^{
		sDevice = [[MTLSoftwareDevice alloc] init];
	});
	return sDevice;
}

- (instancetype)init
{
	self = [super init];
	if (self) {
		_cDevice = mtl_device_create();
		_name = [NSString stringWithUTF8String:mtl_device_name(_cDevice)];
	}
	return self;
}

- (void)dealloc
{
	if (_cDevice) {
		mtl_device_release(_cDevice);
	}
}

- (NSString *)name
{
	return _name;
}

- (uint64_t)registryID
{
	return 0x10001;
}

- (MTLSize)maxThreadsPerThreadgroup
{
	return MTLSizeMake(1024, 1024, 64);
}

- (BOOL)isLowPower
{
	return NO;
}

- (BOOL)isHeadless
{
	return NO;
}

- (BOOL)isRemovable
{
	return NO;
}

- (BOOL)hasUnifiedMemory
{
	return YES;
}

- (uint64_t)recommendedMaxWorkingSetSize
{
	return 2147483648ULL; /* 2 GB */
}

- (id<MTLCommandQueue>)newCommandQueue
{
	return [[MTLSoftwareCommandQueue alloc] initWithDevice:self];
}

- (id<MTLCommandQueue>)newCommandQueueWithMaxCommandBufferCount:(NSUInteger)maxCommandBufferCount
{
	(void)maxCommandBufferCount;
	return [self newCommandQueue];
}

- (id<MTLBuffer>)newBufferWithLength:(NSUInteger)length options:(MTLResourceOptions)options
{
	return [[MTLSoftwareBuffer alloc] initWithDevice:self length:length options:options];
}

- (id<MTLBuffer>)newBufferWithBytes:(const void *)pointer length:(NSUInteger)length options:(MTLResourceOptions)options
{
	return [[MTLSoftwareBuffer alloc] initWithDevice:self bytes:pointer length:length options:options];
}

- (id<MTLBuffer>)newBufferWithBytesNoCopy:(void *)pointer length:(NSUInteger)length options:(MTLResourceOptions)options deallocator:(void (^)(void *, NSUInteger))deallocator
{
	return [[MTLSoftwareBuffer alloc] initWithDevice:self bytesNoCopy:pointer length:length options:options deallocator:deallocator];
}

- (id<MTLTexture>)newTextureWithDescriptor:(MTLTextureDescriptor *)descriptor
{
	return [[MTLSoftwareTexture alloc] initWithDevice:self descriptor:descriptor];
}

- (id<MTLSamplerState>)newSamplerStateWithDescriptor:(MTLSamplerDescriptor *)descriptor
{
	return [[MTLSoftwareSamplerState alloc] initWithDevice:self descriptor:descriptor];
}

- (id<MTLLibrary>)newDefaultLibrary
{
	return [[MTLSoftwareLibrary alloc] initWithDevice:self];
}

- (id<MTLLibrary>)newLibraryWithFile:(NSString *)filepath error:(__autoreleasing NSError **)error
{
	MTLSoftwareLibrary *lib = [[MTLSoftwareLibrary alloc] initWithDevice:self dylibPath:filepath];
	if (lib == nil && error) {
		*error = [NSError errorWithDomain:@"Metal" code:2 userInfo:@{NSLocalizedDescriptionKey: @"Failed to load library file"}];
	}
	return lib;
}

- (id<MTLLibrary>)newLibraryWithURL:(NSURL *)url error:(__autoreleasing NSError **)error
{
	return [self newLibraryWithFile:[url path] error:error];
}

- (id<MTLLibrary>)newLibraryWithData:(NSData *)data error:(__autoreleasing NSError **)error
{
	char tmplib[] = "/tmp/msldata_XXXXXX.metallib";
	int fd = mkstemps(tmplib, 9);
	if (fd < 0) {
		if (error) {
			*error = [NSError errorWithDomain:@"Metal" code:1 userInfo:@{NSLocalizedDescriptionKey: @"Failed to create temp metallib"}];
		}
		return nil;
	}
	write(fd, [data bytes], [data length]);
	close(fd);

	MTLSoftwareLibrary *lib = [[MTLSoftwareLibrary alloc] initWithDevice:self dylibPath:[NSString stringWithUTF8String:tmplib]];
	unlink(tmplib);
	return lib;
}

- (id<MTLLibrary>)newLibraryWithSource:(NSString *)source options:(MTLCompileOptions *)options error:(__autoreleasing NSError **)error
{
	(void)options;
	char tmpsrc[] = "/tmp/msl_XXXXXX.metal";
	int fd = mkstemps(tmpsrc, 6);
	if (fd < 0) {
		if (error) {
			*error = [NSError errorWithDomain:@"Metal" code:1 userInfo:@{NSLocalizedDescriptionKey: @"Failed to create temp shader source file"}];
		}
		return nil;
	}
	const char *srcBytes = [source UTF8String];
	if (srcBytes) {
		write(fd, srcBytes, strlen(srcBytes));
	}
	close(fd);

	char tmpout[sizeof(tmpsrc) + 16];
	snprintf(tmpout, sizeof(tmpout), "%s.metallib", tmpsrc);

	char cmd[1024];
	snprintf(cmd, sizeof(cmd), "/Library/Developer/Toolchains/Default.xctoolchain/usr/bin/metal \"%s\" -o \"%s\" 2>&1", tmpsrc, tmpout);

	FILE *fp = popen(cmd, "r");
	char logbuf[4096];
	size_t n = 0;
	if (fp) {
		n = fread(logbuf, 1, sizeof(logbuf) - 1, fp);
		logbuf[n] = '\0';
		int status = pclose(fp);
		unlink(tmpsrc);
		if (status != 0) {
			if (error) {
				NSString *diag = [NSString stringWithUTF8String:logbuf];
				*error = [NSError errorWithDomain:@"Metal" code:status userInfo:@{NSLocalizedDescriptionKey: diag ?: @"MSL compilation failed"}];
			}
			unlink(tmpout);
			return nil;
		}
	} else {
		unlink(tmpsrc);
		if (error) {
			*error = [NSError errorWithDomain:@"Metal" code:2 userInfo:@{NSLocalizedDescriptionKey: @"Failed to invoke metal compiler"}];
		}
		return nil;
	}

	MTLSoftwareLibrary *lib = [[MTLSoftwareLibrary alloc] initWithDevice:self dylibPath:[NSString stringWithUTF8String:tmpout]];
	unlink(tmpout);
	return lib;
}

- (id<MTLRenderPipelineState>)newRenderPipelineStateWithDescriptor:(MTLRenderPipelineDescriptor *)descriptor error:(__autoreleasing NSError **)error
{
	(void)error;
	return [[MTLSoftwareRenderPipelineState alloc] initWithDevice:self descriptor:descriptor];
}

- (id<MTLComputePipelineState>)newComputePipelineStateWithDescriptor:(MTLComputePipelineDescriptor *)descriptor options:(MTLPipelineOption)options reflection:(MTLComputePipelineReflection **)reflection error:(__autoreleasing NSError **)error
{
	(void)descriptor; (void)options; (void)reflection; (void)error;
	return nil;
}

- (id<MTLComputePipelineState>)newComputePipelineStateWithFunction:(id<MTLFunction>)computeFunction error:(__autoreleasing NSError **)error
{
	(void)computeFunction; (void)error;
	return nil;
}

- (id<MTLDepthStencilState>)newDepthStencilStateWithDescriptor:(MTLDepthStencilDescriptor *)descriptor
{
	return [[MTLSoftwareDepthStencilState alloc] initWithDevice:self descriptor:descriptor];
}

- (BOOL)supportsFeatureSet:(MTLFeatureSet)featureSet
{
	(void)featureSet;
	return YES;
}

@end

/* ------------------------------------------------------------------------- */
/* Public C APIs                                                             */
/* ------------------------------------------------------------------------- */

#import <Metal/MTLIOAccelDevice.h>

id<MTLDevice> MTLCreateSystemDefaultDevice(void)
{
	const char *forceSW = getenv("MTL_FORCE_SOFTWARE");
	const char *backend = getenv("MTL_DEVICE_BACKEND");
	if ((forceSW && atoi(forceSW) != 0) || (backend && strcmp(backend, "software") == 0)) {
		return [MTLSoftwareDevice sharedDevice];
	}
	MTLIOAccelDevice *accel = [MTLIOAccelDevice defaultDevice];
	if (accel) {
		return accel;
	}
	return [MTLSoftwareDevice sharedDevice];
}

NSArray<id<MTLDevice>> *MTLCopyAllDevices(void)
{
	MTLIOAccelDevice *accel = [MTLIOAccelDevice defaultDevice];
	if (accel) {
		return @[ accel, [MTLSoftwareDevice sharedDevice] ];
	}
	return @[ [MTLSoftwareDevice sharedDevice] ];
}

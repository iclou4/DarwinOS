/*
 * MTLIOAccelDevice.h - ravynOS Metal: IOKit Hardware Accelerator device
 */

#ifndef MTL_IOACCEL_DEVICE_H
#define MTL_IOACCEL_DEVICE_H

#import <Metal/_MTLDevice.h>
#import <IOKit/IOTypes.h>

@interface MTLIOAccelDevice : _MTLDevice {
@protected
	io_service_t _acceleratorPort;
	io_connect_t _acceleratorConnect;
	uint64_t _sharedMemorySize;
	uint64_t _dedicatedMemorySize;
	void *_framebufferAperture;
	NSUInteger _framebufferWidth;
	NSUInteger _framebufferHeight;
	NSUInteger _framebufferPitch;
	BOOL _isHardwareActive;
}

@property (readonly) io_service_t acceleratorPort;
@property (readonly) uint64_t sharedMemorySize;
@property (readonly) uint64_t dedicatedMemorySize;
@property (readonly) BOOL isHardwareActive;

+ (nullable instancetype)defaultDevice;
- (nullable instancetype)initWithAcceleratorPort:(io_service_t)port;

- (void)presentPixels:(const void *)pixels width:(NSUInteger)width height:(NSUInteger)height pitch:(NSUInteger)pitch;

@end

#endif /* MTL_IOACCEL_DEVICE_H */

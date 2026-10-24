/*
 * MTLIOAccelResource.h - ravynOS Metal: IOKit Hardware Accelerator resource base
 */

#ifndef MTL_IOACCEL_RESOURCE_H
#define MTL_IOACCEL_RESOURCE_H

#import <Foundation/Foundation.h>
#import <Metal/MTLResource.h>

@class MTLIOAccelDevice;

@interface MTLIOAccelResource : NSObject <MTLResource> {
@protected
	MTLIOAccelDevice *_device;
	NSString *_label;
	MTLResourceOptions _options;
	MTLStorageMode _storageMode;
	MTLCPUCacheMode _cpuCacheMode;
	MTLHazardTrackingMode _hazardTrackingMode;
	void *_virtualAddress;
	NSUInteger _length;
	BOOL _isHardwareMapped;
}

@property (nullable, copy) NSString *label;
@property (readonly) id<MTLDevice> device;
@property (readonly) MTLCPUCacheMode cpuCacheMode;
@property (readonly) MTLStorageMode storageMode;
@property (readonly) MTLHazardTrackingMode hazardTrackingMode;
@property (readonly) MTLResourceOptions resourceOptions;
@property (readonly) void *virtualAddress;
@property (readonly) BOOL isHardwareMapped;

- (instancetype)initWithDevice:(MTLIOAccelDevice *)device options:(MTLResourceOptions)options length:(NSUInteger)length;
- (MTLPurgeableState)setPurgeableState:(MTLPurgeableState)state;
- (BOOL)isAliasable;
- (void)makeAliasable;

@end

#endif /* MTL_IOACCEL_RESOURCE_H */

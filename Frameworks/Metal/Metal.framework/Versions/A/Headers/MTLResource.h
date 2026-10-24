/*
 * MTLResource.h - ravynOS Metal: resource base protocol & options
 */

#ifndef MTL_RESOURCE_H
#define MTL_RESOURCE_H

#import <Metal/MTLDefines.h>

typedef NS_OPTIONS(NSUInteger, MTLResourceOptions) {
	MTLResourceCPUCacheModeDefaultCache  = (0UL << 0),
	MTLResourceCPUCacheModeWriteCombined = (1UL << 0),

	MTLResourceStorageModeShared     = (0UL << 4),
	MTLResourceStorageModeManaged    = (1UL << 4),
	MTLResourceStorageModePrivate    = (2UL << 4),
	MTLResourceStorageModeMemoryless = (3UL << 4),

	MTLResourceHazardTrackingModeDefault   = (0UL << 8),
	MTLResourceHazardTrackingModeUntracked = (1UL << 8),
	MTLResourceHazardTrackingModeTracked   = (2UL << 8),
};

typedef NS_ENUM(NSUInteger, MTLCPUCacheMode) {
	MTLCPUCacheModeDefaultCache  = 0,
	MTLCPUCacheModeWriteCombined = 1,
};

typedef NS_ENUM(NSUInteger, MTLStorageMode) {
	MTLStorageModeShared     = 0,
	MTLStorageModeManaged    = 1,
	MTLStorageModePrivate    = 2,
	MTLStorageModeMemoryless = 3,
};

typedef MTLStorageMode MTLResourceStorageMode;

typedef NS_ENUM(NSUInteger, MTLHazardTrackingMode) {
	MTLHazardTrackingModeDefault   = 0,
	MTLHazardTrackingModeUntracked = 1,
	MTLHazardTrackingModeTracked   = 2,
};

typedef NS_OPTIONS(NSUInteger, MTLPurgeableState) {
	MTLPurgeableStateKeepCurrent = 1,
	MTLPurgeableStateNonVolatile = 2,
	MTLPurgeableStateVolatile    = 3,
	MTLPurgeableStateEmpty       = 4,
};

#define MTLResourceCPUCacheModeShift 0
#define MTLResourceStorageModeShift 4
#define MTLResourceHazardTrackingModeShift 8

#ifdef __OBJC__
#import <Foundation/Foundation.h>

@protocol MTLDevice;
@protocol MTLHeap;

@protocol MTLResource <NSObject>
@property (nullable, copy) NSString *label;
@property (readonly) id<MTLDevice> device;
@property (readonly) MTLCPUCacheMode cpuCacheMode;
@property (readonly) MTLStorageMode storageMode;
@property (readonly) MTLHazardTrackingMode hazardTrackingMode;
@property (readonly) MTLResourceOptions resourceOptions;

- (MTLPurgeableState)setPurgeableState:(MTLPurgeableState)state;
- (BOOL)isAliasable;
- (void)makeAliasable;
@end
#endif /* __OBJC__ */

#endif /* MTL_RESOURCE_H */

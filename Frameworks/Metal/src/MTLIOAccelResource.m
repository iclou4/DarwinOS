/*
 * MTLIOAccelResource.m - ravynOS Metal: IOKit Hardware Accelerator resource base
 */

#import <Metal/MTLIOAccelResource.h>
#import <Metal/MTLIOAccelDevice.h>
#include <stdlib.h>

@implementation MTLIOAccelResource

@synthesize label = _label;
@synthesize device = _device;
@synthesize cpuCacheMode = _cpuCacheMode;
@synthesize storageMode = _storageMode;
@synthesize hazardTrackingMode = _hazardTrackingMode;
@synthesize resourceOptions = _options;
@synthesize virtualAddress = _virtualAddress;
@synthesize isHardwareMapped = _isHardwareMapped;

- (instancetype)initWithDevice:(MTLIOAccelDevice *)device options:(MTLResourceOptions)options length:(NSUInteger)length
{
	self = [super init];
	if (self) {
		_device = device;
		_options = options;
		_length = length;
		_storageMode = (options >> MTLResourceStorageModeShift) & 0x0F;
		_cpuCacheMode = (options >> MTLResourceCPUCacheModeShift) & 0x0F;
		_hazardTrackingMode = (options >> MTLResourceHazardTrackingModeShift) & 0x03;
		_isHardwareMapped = YES;
		_virtualAddress = malloc(length > 0 ? length : 1);
		if (_virtualAddress) {
			memset(_virtualAddress, 0, length);
		}
	}
	return self;
}

- (void)dealloc
{
	if (_virtualAddress) {
		free(_virtualAddress);
		_virtualAddress = NULL;
	}
	[super dealloc];
}

- (MTLPurgeableState)setPurgeableState:(MTLPurgeableState)state
{
	return MTLPurgeableStateNonVolatile;
}

- (BOOL)isAliasable
{
	return NO;
}

- (void)makeAliasable
{
}

- (NSUInteger)allocatedSize
{
	return _length;
}

- (id<MTLHeap>)heap
{
	return nil;
}

- (NSUInteger)heapOffset
{
	return 0;
}

@end

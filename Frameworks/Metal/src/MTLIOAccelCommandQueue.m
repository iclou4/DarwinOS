/*
 * MTLIOAccelCommandQueue.m - ravynOS Metal: IOKit Hardware Accelerator command queue
 */

#import <Metal/MTLIOAccelCommandQueue.h>
#import <Metal/MTLIOAccelCommandBuffer.h>
#import <Metal/MTLIOAccelDevice.h>

@implementation MTLIOAccelCommandQueue

@synthesize label = _label;
@synthesize device = _device;

- (instancetype)initWithDevice:(MTLIOAccelDevice *)device maxCommandBufferCount:(NSUInteger)maxCount
{
	self = [super init];
	if (self) {
		_device = device;
		_maxCommandBufferCount = maxCount > 0 ? maxCount : 64;
	}
	return self;
}

- (id<MTLCommandBuffer>)commandBuffer
{
	return [[MTLIOAccelCommandBuffer alloc] initWithQueue:self retainedReferences:YES];
}

- (id<MTLCommandBuffer>)commandBufferWithUnretainedReferences
{
	return [[MTLIOAccelCommandBuffer alloc] initWithQueue:self retainedReferences:NO];
}

- (void)insertDebugCaptureBoundary
{
}

@end

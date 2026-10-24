/*
 * MTLIOAccelCommandBuffer.m - ravynOS Metal: IOKit Hardware Accelerator command buffer
 */

#import <Metal/MTLIOAccelCommandBuffer.h>
#import <Metal/MTLIOAccelCommandQueue.h>
#import <Metal/MTLIOAccelDevice.h>
#import <Metal/MTLIOAccelRenderCommandEncoder.h>
#import <Metal/MTLIOAccelTexture.h>
#import <Metal/MTLDrawable.h>

@implementation MTLIOAccelCommandBuffer

@synthesize label = _label;
@synthesize status = _status;
@synthesize error = _error;

- (instancetype)initWithQueue:(MTLIOAccelCommandQueue *)queue retainedReferences:(BOOL)retained
{
	self = [super init];
	if (self) {
		_queue = queue;
		_status = MTLCommandBufferStatusNotEnqueued;
		_drawables = [NSMutableArray new];
		_scheduledHandlers = [NSMutableArray new];
		_completedHandlers = [NSMutableArray new];
		_commands = [NSMutableArray new];
	}
	return self;
}

- (id<MTLDevice>)device
{
	return [_queue device];
}

- (id<MTLCommandQueue>)commandQueue
{
	return _queue;
}

- (BOOL)retainedReferences
{
	return YES;
}

- (void)enqueue
{
	_status = MTLCommandBufferStatusEnqueued;
}

- (void)commit
{
	_status = MTLCommandBufferStatusCommitted;

	for (MTLCommandBufferHandler handler in _scheduledHandlers) {
		handler(self);
	}
	_status = MTLCommandBufferStatusScheduled;

	// Present registered drawables and scan out to IOKit / WindowServer
	MTLIOAccelDevice *dev = (MTLIOAccelDevice *)[self device];
	for (id<MTLDrawable> drawable in _drawables) {
		[drawable present];

		// If drawable has a texture, push it directly to the IOKit accelerator scanout
		if ([drawable respondsToSelector:@selector(texture)]) {
			id<MTLTexture> tex = [((id<CAMetalDrawable>)drawable) texture];
			if (tex) {
				void *pixels = [((id)tex) contents];
				if (pixels) {
					[dev presentPixels:pixels
						     width:[tex width]
						    height:[tex height]
						     pitch:[tex width] * 4];
				}
			}
		}
	}

	_status = MTLCommandBufferStatusCompleted;
	for (MTLCommandBufferHandler handler in _completedHandlers) {
		handler(self);
	}
}

- (void)addScheduledHandler:(MTLCommandBufferHandler)block
{
	[_scheduledHandlers addObject:[block copy]];
}

- (void)addCompletedHandler:(MTLCommandBufferHandler)block
{
	[_completedHandlers addObject:[block copy]];
}

- (void)waitUntilScheduled
{
}

- (void)waitUntilCompleted
{
}

- (void)presentDrawable:(id<MTLDrawable>)drawable
{
	if (drawable && ![_drawables containsObject:drawable]) {
		[_drawables addObject:drawable];
	}
}

- (void)presentDrawable:(id<MTLDrawable>)drawable atTime:(CFTimeInterval)presentationTime
{
	(void)presentationTime;
	[self presentDrawable:drawable];
}

- (void)presentDrawable:(id<MTLDrawable>)drawable afterMinimumDuration:(CFTimeInterval)duration
{
	(void)duration;
	[self presentDrawable:drawable];
}

- (id<MTLRenderCommandEncoder>)renderCommandEncoderWithDescriptor:(MTLRenderPassDescriptor *)descriptor
{
	return [[MTLIOAccelRenderCommandEncoder alloc] initWithCommandBuffer:self descriptor:descriptor];
}

- (id<MTLComputeCommandEncoder>)computeCommandEncoder
{
	return nil;
}

- (id<MTLBlitCommandEncoder>)blitCommandEncoder
{
	return nil;
}

@end

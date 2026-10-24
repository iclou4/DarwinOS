/*
 * MTLSoftwareCommandBuffer.m - ravynOS Metal: command buffer implementation
 */

#import "MTLSoftwareCommandBuffer.h"
#import "MTLSoftwareRenderCommandEncoder.h"
#import "MTLSoftwareTexture.h"
#import <Metal/MTLRenderPass.h>
#import <Metal/MTLDrawable.h>
#import <Metal/MTLCommandQueue.h>
#import <Metal/MTLDevice.h>

@implementation MTLSoftwareCommandBuffer

@synthesize label = _label;
@synthesize status = _status;
@synthesize error = _error;
@synthesize cCommandBuffer = _cCommandBuffer;

- (instancetype)initWithQueue:(id<MTLCommandQueue>)queue cCommandBuffer:(mtl_command_buffer_t *)cBuffer
{
	self = [super init];
	if (self) {
		_queue = queue;
		_cCommandBuffer = cBuffer;
		_status = MTLCommandBufferStatusNotEnqueued;
		_drawablesToPresent = [NSMutableArray new];
		_completedHandlers = [NSMutableArray new];
		_scheduledHandlers = [NSMutableArray new];
	}
	return self;
}

- (void)dealloc
{
	if (_cCommandBuffer) {
		mtl_command_buffer_release(_cCommandBuffer);
	}
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

	/* Execute all recorded commands in the command buffer */
	mtl_command_buffer_commit(_cCommandBuffer);

	/* Present any registered drawables */
	for (id<MTLDrawable> drawable in _drawablesToPresent) {
		[drawable present];
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

- (id<MTLRenderCommandEncoder>)renderCommandEncoderWithDescriptor:(MTLRenderPassDescriptor *)renderPassDescriptor
{
	mtl_render_pass_descriptor cPass;
	memset(&cPass, 0, sizeof(cPass));

	MTLRenderPassColorAttachmentDescriptor *color0 = renderPassDescriptor.colorAttachments[0];
	if (color0 && [color0.texture isKindOfClass:[MTLSoftwareTexture class]]) {
		cPass.color_attachment = ((MTLSoftwareTexture *)color0.texture).cTexture;
		cPass.color_load_action = color0.loadAction;
		cPass.color_store_action = color0.storeAction;
		cPass.clear_color = mtl_make_float4(
			(float)color0.clearColor.red,
			(float)color0.clearColor.green,
			(float)color0.clearColor.blue,
			(float)color0.clearColor.alpha);
	}

	MTLRenderPassDepthAttachmentDescriptor *depth = renderPassDescriptor.depthAttachment;
	if (depth && [depth.texture isKindOfClass:[MTLSoftwareTexture class]]) {
		cPass.depth_attachment = ((MTLSoftwareTexture *)depth.texture).cTexture;
		cPass.depth_load_action = depth.loadAction;
		cPass.depth_store_action = depth.storeAction;
		cPass.clear_depth = (float)depth.clearDepth;
	}

	mtl_render_encoder_t *cEncoder = mtl_command_buffer_render_encoder(_cCommandBuffer, &cPass);
	return [[MTLSoftwareRenderCommandEncoder alloc] initWithCommandBuffer:self descriptor:renderPassDescriptor cEncoder:cEncoder];
}

- (id<MTLComputeCommandEncoder>)computeCommandEncoder
{
	return nil;
}

- (id<MTLBlitCommandEncoder>)blitCommandEncoder
{
	return nil;
}

- (void)presentDrawable:(id<MTLDrawable>)drawable
{
	if (drawable) {
		[_drawablesToPresent addObject:drawable];
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

@end

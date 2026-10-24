/*
 * MTLIOAccelCommandBuffer.h - ravynOS Metal: IOKit Hardware Accelerator command buffer
 */

#ifndef MTL_IOACCEL_COMMAND_BUFFER_H
#define MTL_IOACCEL_COMMAND_BUFFER_H

#import <Foundation/Foundation.h>
#import <Metal/MTLCommandBuffer.h>
#import <Metal/MTLCommandQueue.h>
#import <Metal/MTLDrawable.h>

@class MTLIOAccelCommandQueue;
@class MTLIOAccelDevice;

@interface MTLIOAccelCommandBuffer : NSObject <MTLCommandBuffer> {
@protected
	MTLIOAccelCommandQueue *_queue;
	NSString *_label;
	MTLCommandBufferStatus _status;
	NSError *_error;
	NSMutableArray *_drawables;
	NSMutableArray *_scheduledHandlers;
	NSMutableArray *_completedHandlers;
	NSMutableArray *_commands;
}

@property (readonly) id<MTLDevice> device;
@property (readonly) id<MTLCommandQueue> commandQueue;
@property (readonly) BOOL retainedReferences;
@property (nullable, copy) NSString *label;
@property (readonly) MTLCommandBufferStatus status;
@property (nullable, readonly) NSError *error;

- (instancetype)initWithQueue:(MTLIOAccelCommandQueue *)queue retainedReferences:(BOOL)retained;

- (void)enqueue;
- (void)commit;
- (void)addScheduledHandler:(MTLCommandBufferHandler)block;
- (void)addCompletedHandler:(MTLCommandBufferHandler)block;
- (void)presentDrawable:(id<MTLDrawable>)drawable;
- (void)presentDrawable:(id<MTLDrawable>)drawable atTime:(CFTimeInterval)presentationTime;
- (void)presentDrawable:(id<MTLDrawable>)drawable afterMinimumDuration:(CFTimeInterval)duration;
- (void)waitUntilScheduled;
- (void)waitUntilCompleted;

- (nullable id<MTLRenderCommandEncoder>)renderCommandEncoderWithDescriptor:(MTLRenderPassDescriptor *)descriptor;
- (nullable id<MTLComputeCommandEncoder>)computeCommandEncoder;
- (nullable id<MTLBlitCommandEncoder>)blitCommandEncoder;

@end

#endif /* MTL_IOACCEL_COMMAND_BUFFER_H */

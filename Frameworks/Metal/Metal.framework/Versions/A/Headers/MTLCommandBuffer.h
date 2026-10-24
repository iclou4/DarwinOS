/*
 * MTLCommandBuffer.h - ravynOS Metal: command buffer protocol and status
 */

#ifndef MTL_COMMAND_BUFFER_H
#define MTL_COMMAND_BUFFER_H

#import <Metal/MTLDefines.h>

typedef NS_ENUM(NSUInteger, MTLCommandBufferStatus) {
	MTLCommandBufferStatusNotEnqueued = 0,
	MTLCommandBufferStatusEnqueued    = 1,
	MTLCommandBufferStatusCommitted   = 2,
	MTLCommandBufferStatusScheduled   = 3,
	MTLCommandBufferStatusCompleted   = 4,
	MTLCommandBufferStatusError       = 5,
};

typedef NS_ENUM(NSUInteger, MTLCommandBufferError) {
	MTLCommandBufferErrorNone               = 0,
	MTLCommandBufferErrorInternal           = 1,
	MTLCommandBufferErrorTimeout            = 2,
	MTLCommandBufferErrorPageFault          = 3,
	MTLCommandBufferErrorBlacklisted        = 4,
	MTLCommandBufferErrorNotPermitted       = 7,
	MTLCommandBufferErrorOutOfMemory        = 8,
	MTLCommandBufferErrorInvalidResource    = 9,
	MTLCommandBufferErrorMemoryless         = 10,
	MTLCommandBufferErrorDeviceRemoved      = 11,
	MTLCommandBufferErrorStackOverflow      = 12,
};

#ifdef __OBJC__
#import <Foundation/Foundation.h>

@protocol MTLDevice;
@protocol MTLCommandQueue;
@protocol MTLDrawable;
@protocol MTLRenderCommandEncoder;
@protocol MTLComputeCommandEncoder;
@protocol MTLBlitCommandEncoder;
@protocol MTLCommandBuffer;
@class MTLRenderPassDescriptor;

typedef void (^MTLCommandBufferHandler)(id<MTLCommandBuffer>);

@protocol MTLCommandBuffer <NSObject>
@property (readonly) id<MTLDevice> device;
@property (readonly) id<MTLCommandQueue> commandQueue;
@property (readonly) BOOL retainedReferences;
@property (nullable, copy) NSString *label;
@property (readonly) MTLCommandBufferStatus status;
@property (nullable, readonly) NSError *error;

- (void)enqueue;
- (void)commit;
- (void)addScheduledHandler:(MTLCommandBufferHandler)block;
- (void)addCompletedHandler:(MTLCommandBufferHandler)block;
- (void)waitUntilScheduled;
- (void)waitUntilCompleted;

- (nullable id<MTLRenderCommandEncoder>)renderCommandEncoderWithDescriptor:(MTLRenderPassDescriptor *)renderPassDescriptor;
- (nullable id<MTLComputeCommandEncoder>)computeCommandEncoder;
- (nullable id<MTLBlitCommandEncoder>)blitCommandEncoder;

- (void)presentDrawable:(id<MTLDrawable>)drawable;
- (void)presentDrawable:(id<MTLDrawable>)drawable atTime:(CFTimeInterval)presentationTime;
- (void)presentDrawable:(id<MTLDrawable>)drawable afterMinimumDuration:(CFTimeInterval)duration;
@end
#endif /* __OBJC__ */

#endif /* MTL_COMMAND_BUFFER_H */

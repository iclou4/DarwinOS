/*
 * MTLIOAccelCommandQueue.h - ravynOS Metal: IOKit Hardware Accelerator command queue
 */

#ifndef MTL_IOACCEL_COMMAND_QUEUE_H
#define MTL_IOACCEL_COMMAND_QUEUE_H

#import <Foundation/Foundation.h>
#import <Metal/MTLCommandQueue.h>

@class MTLIOAccelDevice;

@interface MTLIOAccelCommandQueue : NSObject <MTLCommandQueue> {
@protected
	MTLIOAccelDevice *_device;
	NSString *_label;
	NSUInteger _maxCommandBufferCount;
}

@property (nullable, copy) NSString *label;
@property (readonly) id<MTLDevice> device;

- (instancetype)initWithDevice:(MTLIOAccelDevice *)device maxCommandBufferCount:(NSUInteger)maxCount;
- (nullable id<MTLCommandBuffer>)commandBuffer;
- (nullable id<MTLCommandBuffer>)commandBufferWithUnretainedReferences;

@end

#endif /* MTL_IOACCEL_COMMAND_QUEUE_H */

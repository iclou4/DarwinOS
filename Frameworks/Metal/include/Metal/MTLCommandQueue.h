/*
 * MTLCommandQueue.h - ravynOS Metal: command queue protocol
 */

#ifndef MTL_COMMAND_QUEUE_H
#define MTL_COMMAND_QUEUE_H

#import <Foundation/Foundation.h>

@protocol MTLDevice;
@protocol MTLCommandBuffer;

@protocol MTLCommandQueue <NSObject>
@property (nullable, copy) NSString *label;
@property (readonly) id<MTLDevice> device;

- (nullable id<MTLCommandBuffer>)commandBuffer;
- (nullable id<MTLCommandBuffer>)commandBufferWithUnretainedReferences;
- (void)insertDebugCaptureBoundary;
@end

#endif /* MTL_COMMAND_QUEUE_H */

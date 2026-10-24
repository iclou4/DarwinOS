/*
 * MTLSoftwareCommandBuffer.h - ravynOS Metal: command buffer implementation
 */

#ifndef MTL_SOFTWARE_COMMAND_BUFFER_H
#define MTL_SOFTWARE_COMMAND_BUFFER_H

#import <Foundation/Foundation.h>
#import <Metal/MTLCommandBuffer.h>
#import "mtl_internal.h"

@interface MTLSoftwareCommandBuffer : NSObject <MTLCommandBuffer> {
@public
	id<MTLCommandQueue> _queue;
	NSString *_label;
	mtl_command_buffer_t *_cCommandBuffer;
	MTLCommandBufferStatus _status;
	NSError *_error;
	NSMutableArray *_drawablesToPresent;
	NSMutableArray *_completedHandlers;
	NSMutableArray *_scheduledHandlers;
}

@property (readonly) mtl_command_buffer_t *cCommandBuffer;

- (instancetype)initWithQueue:(id<MTLCommandQueue>)queue cCommandBuffer:(mtl_command_buffer_t *)cBuffer;

@end

#endif /* MTL_SOFTWARE_COMMAND_BUFFER_H */

/*
 * MTLSoftwareCommandQueue.h - ravynOS Metal: command queue implementation
 */

#ifndef MTL_SOFTWARE_COMMAND_QUEUE_H
#define MTL_SOFTWARE_COMMAND_QUEUE_H

#import <Foundation/Foundation.h>
#import <Metal/MTLCommandQueue.h>
#import "mtl_internal.h"

@interface MTLSoftwareCommandQueue : NSObject <MTLCommandQueue> {
@public
	id<MTLDevice> _device;
	NSString *_label;
	mtl_command_queue_t *_cQueue;
}

@property (readonly) mtl_command_queue_t *cQueue;

- (instancetype)initWithDevice:(id<MTLDevice>)device;

@end

#endif /* MTL_SOFTWARE_COMMAND_QUEUE_H */

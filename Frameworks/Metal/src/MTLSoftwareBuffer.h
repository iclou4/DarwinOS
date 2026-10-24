/*
 * MTLSoftwareBuffer.h - ravynOS Metal: buffer implementation
 */

#ifndef MTL_SOFTWARE_BUFFER_H
#define MTL_SOFTWARE_BUFFER_H

#import <Foundation/Foundation.h>
#import <Metal/MTLBuffer.h>
#import "mtl_internal.h"

@interface MTLSoftwareBuffer : NSObject <MTLBuffer> {
@public
	id<MTLDevice> _device;
	NSUInteger _length;
	void *_contents;
	BOOL _ownsMemory;
	void (^_deallocator)(void *, NSUInteger);
	mtl_buffer_t *_cBuffer;
	NSString *_label;
	MTLResourceOptions _options;
}

@property (readonly) mtl_buffer_t *cBuffer;

- (instancetype)initWithDevice:(id<MTLDevice>)device length:(NSUInteger)length options:(MTLResourceOptions)options;
- (instancetype)initWithDevice:(id<MTLDevice>)device bytes:(const void *)pointer length:(NSUInteger)length options:(MTLResourceOptions)options;
- (instancetype)initWithDevice:(id<MTLDevice>)device bytesNoCopy:(void *)pointer length:(NSUInteger)length options:(MTLResourceOptions)options deallocator:(void (^)(void *pointer, NSUInteger length))deallocator;

@end

#endif /* MTL_SOFTWARE_BUFFER_H */

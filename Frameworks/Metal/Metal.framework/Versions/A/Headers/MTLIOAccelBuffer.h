/*
 * MTLIOAccelBuffer.h - ravynOS Metal: IOKit Hardware Accelerator buffer
 */

#ifndef MTL_IOACCEL_BUFFER_H
#define MTL_IOACCEL_BUFFER_H

#import <Metal/MTLBuffer.h>
#import <Metal/MTLIOAccelResource.h>

@interface MTLIOAccelBuffer : MTLIOAccelResource <MTLBuffer> {
@protected
	void *_pointer;
}

@property (readonly) NSUInteger length;

- (instancetype)initWithDevice:(MTLIOAccelDevice *)device length:(NSUInteger)length options:(MTLResourceOptions)options;
- (instancetype)initWithDevice:(MTLIOAccelDevice *)device bytes:(const void *)pointer length:(NSUInteger)length options:(MTLResourceOptions)options;

- (void *)contents;
- (void)didModifyRange:(NSRange)range;

@end

#endif /* MTL_IOACCEL_BUFFER_H */

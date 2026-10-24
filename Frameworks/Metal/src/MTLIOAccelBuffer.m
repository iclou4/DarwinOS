/*
 * MTLIOAccelBuffer.m - ravynOS Metal: IOKit Hardware Accelerator buffer implementation
 */

#import <Metal/MTLIOAccelBuffer.h>
#import <Metal/MTLIOAccelDevice.h>
#include <string.h>

@implementation MTLIOAccelBuffer

- (instancetype)initWithDevice:(MTLIOAccelDevice *)device length:(NSUInteger)length options:(MTLResourceOptions)options
{
	self = [super initWithDevice:device options:options length:length];
	if (self) {
		_pointer = _virtualAddress;
	}
	return self;
}

- (instancetype)initWithDevice:(MTLIOAccelDevice *)device bytes:(const void *)pointer length:(NSUInteger)length options:(MTLResourceOptions)options
{
	self = [super initWithDevice:device options:options length:length];
	if (self) {
		_pointer = _virtualAddress;
		if (pointer && _pointer) {
			memcpy(_pointer, pointer, length);
		}
	}
	return self;
}

- (NSUInteger)length
{
	return _length;
}

- (void *)contents
{
	return _pointer;
}

- (void)didModifyRange:(NSRange)range
{
	(void)range;
}

- (void)addDebugMarker:(NSString *)marker range:(NSRange)range
{
	(void)marker; (void)range;
}

- (void)removeAllDebugMarkers
{
}

- (id<MTLTexture>)newTextureWithDescriptor:(MTLTextureDescriptor *)descriptor offset:(NSUInteger)offset bytesPerRow:(NSUInteger)bytesPerRow
{
	(void)descriptor; (void)offset; (void)bytesPerRow;
	return nil;
}

@end

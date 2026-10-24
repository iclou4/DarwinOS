/*
 * MTLIOAccelTexture.m - ravynOS Metal: IOKit Hardware Accelerator texture implementation
 */

#import <Metal/MTLIOAccelTexture.h>
#import <Metal/MTLIOAccelDevice.h>
#include <string.h>

@implementation MTLIOAccelTexture

@synthesize textureType = _textureType;
@synthesize pixelFormat = _pixelFormat;
@synthesize width = _width;
@synthesize height = _height;
@synthesize depth = _depth;
@synthesize mipmapLevelCount = _mipmapLevelCount;
@synthesize sampleCount = _sampleCount;
@synthesize arrayLength = _arrayLength;
@synthesize usage = _usage;
@synthesize framebufferOnly = _framebufferOnly;

- (instancetype)initWithDevice:(MTLIOAccelDevice *)device descriptor:(MTLTextureDescriptor *)descriptor
{
	NSUInteger bpp = 4;
	switch (descriptor.pixelFormat) {
		case MTLPixelFormatRGBA8Unorm:
		case MTLPixelFormatBGRA8Unorm:
		case MTLPixelFormatRGBA8Unorm_sRGB:
		case MTLPixelFormatBGRA8Unorm_sRGB:
			bpp = 4;
			break;
		case MTLPixelFormatRGBA16Float:
			bpp = 8;
			break;
		case MTLPixelFormatRGBA32Float:
			bpp = 16;
			break;
		case MTLPixelFormatR8Unorm:
			bpp = 1;
			break;
		default:
			bpp = 4;
			break;
	}

	NSUInteger rowBytes = descriptor.width * bpp;
	NSUInteger totalBytes = rowBytes * descriptor.height;

	self = [super initWithDevice:device options:descriptor.resourceOptions length:totalBytes];
	if (self) {
		_textureType = descriptor.textureType;
		_pixelFormat = descriptor.pixelFormat;
		_width = descriptor.width;
		_height = descriptor.height;
		_depth = descriptor.depth;
		_mipmapLevelCount = descriptor.mipmapLevelCount;
		_sampleCount = descriptor.sampleCount;
		_arrayLength = descriptor.arrayLength;
		_usage = descriptor.usage;
		_framebufferOnly = NO;
		_bytesPerRow = rowBytes;
		_pixelData = _virtualAddress;
	}
	return self;
}

- (void *)contents
{
	return _pixelData;
}

- (NSUInteger)bytesPerRow
{
	return _bytesPerRow;
}

- (id<MTLResource>)rootResource
{
	return self;
}

- (id<MTLTexture>)parentTexture
{
	return nil;
}

- (NSUInteger)parentRelativeLevel
{
	return 0;
}

- (NSUInteger)parentRelativeSlice
{
	return 0;
}

- (id<MTLBuffer>)buffer
{
	return nil;
}

- (NSUInteger)bufferOffset
{
	return 0;
}

- (NSUInteger)bufferBytesPerRow
{
	return _bytesPerRow;
}

- (BOOL)allowGPUOptimizedContents
{
	return YES;
}

- (void)getBytes:(void *)pixelBytes bytesPerRow:(NSUInteger)bytesPerRow fromRegion:(MTLRegion)region mipmapLevel:(NSUInteger)level
{
	if (!_pixelData || !pixelBytes) return;
	NSUInteger bpp = 4;
	uint8_t *src = (uint8_t *)_pixelData + region.origin.y * _bytesPerRow + region.origin.x * bpp;
	uint8_t *dst = (uint8_t *)pixelBytes;
	NSUInteger copyBytes = region.size.width * bpp;
	for (NSUInteger y = 0; y < region.size.height; y++) {
		memcpy(dst + y * bytesPerRow, src + y * _bytesPerRow, copyBytes);
	}
}

- (void)replaceRegion:(MTLRegion)region mipmapLevel:(NSUInteger)level withBytes:(const void *)pixelBytes bytesPerRow:(NSUInteger)bytesPerRow
{
	if (!_pixelData || !pixelBytes) return;
	NSUInteger bpp = 4;
	uint8_t *dst = (uint8_t *)_pixelData + region.origin.y * _bytesPerRow + region.origin.x * bpp;
	const uint8_t *src = (const uint8_t *)pixelBytes;
	NSUInteger copyBytes = region.size.width * bpp;
	for (NSUInteger y = 0; y < region.size.height; y++) {
		memcpy(dst + y * _bytesPerRow, src + y * bytesPerRow, copyBytes);
	}
}

- (void)getBytes:(void *)pixelBytes bytesPerRow:(NSUInteger)bytesPerRow bytesPerImage:(NSUInteger)bytesPerImage fromRegion:(MTLRegion)region mipmapLevel:(NSUInteger)level slice:(NSUInteger)slice
{
	(void)bytesPerImage; (void)slice;
	[self getBytes:pixelBytes bytesPerRow:bytesPerRow fromRegion:region mipmapLevel:level];
}

- (void)replaceRegion:(MTLRegion)region mipmapLevel:(NSUInteger)level slice:(NSUInteger)slice withBytes:(const void *)pixelBytes bytesPerRow:(NSUInteger)bytesPerRow bytesPerImage:(NSUInteger)bytesPerImage
{
	(void)bytesPerImage; (void)slice;
	[self replaceRegion:region mipmapLevel:level withBytes:pixelBytes bytesPerRow:bytesPerRow];
}

- (id<MTLTexture>)newTextureViewWithPixelFormat:(MTLPixelFormat)pixelFormat
{
	(void)pixelFormat;
	return nil;
}

@end

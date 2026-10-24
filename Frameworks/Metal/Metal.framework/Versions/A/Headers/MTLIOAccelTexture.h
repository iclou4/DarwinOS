/*
 * MTLIOAccelTexture.h - ravynOS Metal: IOKit Hardware Accelerator texture
 */

#ifndef MTL_IOACCEL_TEXTURE_H
#define MTL_IOACCEL_TEXTURE_H

#import <Metal/MTLTexture.h>
#import <Metal/MTLIOAccelResource.h>

@interface MTLIOAccelTexture : MTLIOAccelResource <MTLTexture> {
@protected
	MTLTextureType _textureType;
	MTLPixelFormat _pixelFormat;
	NSUInteger _width;
	NSUInteger _height;
	NSUInteger _depth;
	NSUInteger _mipmapLevelCount;
	NSUInteger _sampleCount;
	NSUInteger _arrayLength;
	MTLTextureUsage _usage;
	BOOL _framebufferOnly;
	void *_pixelData;
	NSUInteger _bytesPerRow;
}

@property (readonly) id<MTLResource> rootResource;
@property (nullable, readonly) id<MTLTexture> parentTexture;
@property (readonly) NSUInteger parentRelativeLevel;
@property (readonly) NSUInteger parentRelativeSlice;
@property (nullable, readonly) id<MTLBuffer> buffer;
@property (readonly) NSUInteger bufferOffset;
@property (readonly) NSUInteger bufferBytesPerRow;
@property (readonly) MTLTextureType textureType;
@property (readonly) MTLPixelFormat pixelFormat;
@property (readonly) NSUInteger width;
@property (readonly) NSUInteger height;
@property (readonly) NSUInteger depth;
@property (readonly) NSUInteger mipmapLevelCount;
@property (readonly) NSUInteger sampleCount;
@property (readonly) NSUInteger arrayLength;
@property (readonly) MTLTextureUsage usage;
@property (readonly, getter=isFramebufferOnly) BOOL framebufferOnly;
@property (readonly) BOOL allowGPUOptimizedContents;

- (instancetype)initWithDevice:(MTLIOAccelDevice *)device descriptor:(MTLTextureDescriptor *)descriptor;

- (void *)contents;
- (NSUInteger)bytesPerRow;

- (void)getBytes:(void *)pixelBytes bytesPerRow:(NSUInteger)bytesPerRow fromRegion:(MTLRegion)region mipmapLevel:(NSUInteger)level;
- (void)replaceRegion:(MTLRegion)region mipmapLevel:(NSUInteger)level withBytes:(const void *)pixelBytes bytesPerRow:(NSUInteger)bytesPerRow;

@end

#endif /* MTL_IOACCEL_TEXTURE_H */

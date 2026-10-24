/*
 * MTLTexture.h - ravynOS Metal: texture protocol and descriptors
 */

#ifndef MTL_TEXTURE_H
#define MTL_TEXTURE_H

#import <Metal/MTLDefines.h>
#import <Metal/MTLResource.h>
#import <Metal/MTLTypes.h>
#import <Metal/MTLPixelFormat.h>

typedef NS_ENUM(NSUInteger, MTLTextureType) {
	MTLTextureType1D               = 0,
	MTLTextureType1DArray          = 1,
	MTLTextureType2D               = 2,
	MTLTextureType2DArray          = 3,
	MTLTextureType2DMultisample    = 4,
	MTLTextureTypeCube             = 5,
	MTLTextureTypeCubeArray        = 6,
	MTLTextureType3D               = 7,
	MTLTextureType2DMultisampleArray = 8,
	MTLTextureTypeTextureBuffer    = 9
};

typedef NS_OPTIONS(NSUInteger, MTLTextureUsage) {
	MTLTextureUsageUnknown         = 0x0000,
	MTLTextureUsageShaderRead      = 0x0001,
	MTLTextureUsageShaderWrite     = 0x0002,
	MTLTextureUsageRenderTarget    = 0x0004,
	MTLTextureUsagePixelFormatView = 0x0010,
};

#ifdef __OBJC__
#import <Foundation/Foundation.h>

@protocol MTLBuffer;

@interface MTLTextureDescriptor : NSObject <NSCopying>
@property (readwrite, nonatomic) MTLTextureType textureType;
@property (readwrite, nonatomic) MTLPixelFormat pixelFormat;
@property (readwrite, nonatomic) NSUInteger width;
@property (readwrite, nonatomic) NSUInteger height;
@property (readwrite, nonatomic) NSUInteger depth;
@property (readwrite, nonatomic) NSUInteger mipmapLevelCount;
@property (readwrite, nonatomic) NSUInteger sampleCount;
@property (readwrite, nonatomic) NSUInteger arrayLength;
@property (readwrite, nonatomic) MTLResourceOptions resourceOptions;
@property (readwrite, nonatomic) MTLCPUCacheMode cpuCacheMode;
@property (readwrite, nonatomic) MTLStorageMode storageMode;
@property (readwrite, nonatomic) MTLTextureUsage usage;

+ (MTLTextureDescriptor *)texture2DDescriptorWithPixelFormat:(MTLPixelFormat)pixelFormat
                                                      width:(NSUInteger)width
                                                     height:(NSUInteger)height
                                                  mipmapped:(BOOL)mipmapped;

+ (MTLTextureDescriptor *)textureCubeDescriptorWithPixelFormat:(MTLPixelFormat)pixelFormat
                                                          size:(NSUInteger)size
                                                     mipmapped:(BOOL)mipmapped;
@end

@protocol MTLTexture <MTLResource>
@property (nullable, readonly) id<MTLResource> rootResource;
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

- (void)getBytes:(void *)pixelBytes
     bytesPerRow:(NSUInteger)bytesPerRow
   bytesPerImage:(NSUInteger)bytesPerImage
      fromRegion:(MTLRegion)region
     mipmapLevel:(NSUInteger)level
           slice:(NSUInteger)slice;

- (void)replaceRegion:(MTLRegion)region
          mipmapLevel:(NSUInteger)level
                slice:(NSUInteger)slice
            withBytes:(const void *)pixelBytes
          bytesPerRow:(NSUInteger)bytesPerRow
        bytesPerImage:(NSUInteger)bytesPerImage;

- (void)getBytes:(void *)pixelBytes
     bytesPerRow:(NSUInteger)bytesPerRow
      fromRegion:(MTLRegion)region
     mipmapLevel:(NSUInteger)level;

- (void)replaceRegion:(MTLRegion)region
          mipmapLevel:(NSUInteger)level
            withBytes:(const void *)pixelBytes
          bytesPerRow:(NSUInteger)bytesPerRow;
@end
#endif /* __OBJC__ */

#endif /* MTL_TEXTURE_H */

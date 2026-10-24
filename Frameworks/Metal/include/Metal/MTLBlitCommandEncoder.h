/*
 * MTLBlitCommandEncoder.h - ravynOS Metal: memory transfer encoder protocol
 */

#ifndef MTL_BLIT_COMMAND_ENCODER_H
#define MTL_BLIT_COMMAND_ENCODER_H

#import <Foundation/Foundation.h>
#import <Metal/MTLCommandEncoder.h>
#import <Metal/MTLTypes.h>

@protocol MTLBuffer;
@protocol MTLTexture;

typedef NS_OPTIONS(NSUInteger, MTLBlitOption) {
	MTLBlitOptionNone                   = 0,
	MTLBlitOptionDepthFromDepthStencil   = 1 << 0,
	MTLBlitOptionStencilFromDepthStencil = 1 << 1,
	MTLBlitOptionRowLinearPVRTC          = 1 << 2
};

@protocol MTLBlitCommandEncoder <MTLCommandEncoder>

- (void)copyFromBuffer:(id<MTLBuffer>)sourceBuffer
          sourceOffset:(NSUInteger)sourceOffset
              toBuffer:(id<MTLBuffer>)destinationBuffer
     destinationOffset:(NSUInteger)destinationOffset
                  size:(NSUInteger)size;

- (void)copyFromBuffer:(id<MTLBuffer>)sourceBuffer
          sourceOffset:(NSUInteger)sourceOffset
     sourceBytesPerRow:(NSUInteger)sourceBytesPerRow
   sourceBytesPerImage:(NSUInteger)sourceBytesPerImage
            sourceSize:(MTLSize)sourceSize
             toTexture:(id<MTLTexture>)destinationTexture
      destinationSlice:(NSUInteger)destinationSlice
      destinationLevel:(NSUInteger)destinationLevel
     destinationOrigin:(MTLOrigin)destinationOrigin;

- (void)copyFromTexture:(id<MTLTexture>)sourceTexture
            sourceSlice:(NSUInteger)sourceSlice
            sourceLevel:(NSUInteger)sourceLevel
           sourceOrigin:(MTLOrigin)sourceOrigin
             sourceSize:(MTLSize)sourceSize
              toTexture:(id<MTLTexture>)destinationTexture
       destinationSlice:(NSUInteger)destinationSlice
       destinationLevel:(NSUInteger)destinationLevel
      destinationOrigin:(MTLOrigin)destinationOrigin;

- (void)copyFromTexture:(id<MTLTexture>)sourceTexture
            sourceSlice:(NSUInteger)sourceSlice
            sourceLevel:(NSUInteger)sourceLevel
           sourceOrigin:(MTLOrigin)sourceOrigin
             sourceSize:(MTLSize)sourceSize
               toBuffer:(id<MTLBuffer>)destinationBuffer
      destinationOffset:(NSUInteger)destinationOffset
 destinationBytesPerRow:(NSUInteger)destinationBytesPerRow
destinationBytesPerImage:(NSUInteger)destinationBytesPerImage;

- (void)fillBuffer:(id<MTLBuffer>)buffer range:(NSRange)range value:(uint8_t)value;

- (void)generateMipmapsForTexture:(id<MTLTexture>)texture;
@end

#endif /* MTL_BLIT_COMMAND_ENCODER_H */

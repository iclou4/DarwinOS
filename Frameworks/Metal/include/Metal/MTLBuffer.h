/*
 * MTLBuffer.h - ravynOS Metal: buffer protocol
 */

#ifndef MTL_BUFFER_H
#define MTL_BUFFER_H

#import <Foundation/Foundation.h>
#import <Metal/MTLResource.h>

@protocol MTLTexture;
@class MTLTextureDescriptor;

@protocol MTLBuffer <MTLResource>
@property (readonly) NSUInteger length;

- (void *)contents;
- (void)didModifyRange:(NSRange)range;
- (nullable id<MTLTexture>)newTextureWithDescriptor:(MTLTextureDescriptor *)descriptor
                                            offset:(NSUInteger)offset
                                       bytesPerRow:(NSUInteger)bytesPerRow;
- (void)addDebugMarker:(NSString *)marker range:(NSRange)range;
- (void)removeAllDebugMarkers;
@end

#endif /* MTL_BUFFER_H */

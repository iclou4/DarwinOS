/*
 * MTLSoftwareTexture.h - ravynOS Metal: texture implementation
 */

#ifndef MTL_SOFTWARE_TEXTURE_H
#define MTL_SOFTWARE_TEXTURE_H

#import <Foundation/Foundation.h>
#import <Metal/MTLTexture.h>
#import "mtl_internal.h"

@interface MTLSoftwareTexture : NSObject <MTLTexture> {
@public
	id<MTLDevice> _device;
	MTLTextureDescriptor *_descriptor;
	mtl_texture_t *_cTexture;
	NSString *_label;
}

@property (readonly) mtl_texture_t *cTexture;

- (instancetype)initWithDevice:(id<MTLDevice>)device descriptor:(MTLTextureDescriptor *)descriptor;
- (void *)contents;

@end

#endif /* MTL_SOFTWARE_TEXTURE_H */

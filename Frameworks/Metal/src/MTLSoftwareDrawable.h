/*
 * MTLSoftwareDrawable.h - ravynOS Metal: software drawable
 */

#ifndef MTL_SOFTWARE_DRAWABLE_H
#define MTL_SOFTWARE_DRAWABLE_H

#import <Foundation/Foundation.h>
#import <Metal/MTLDrawable.h>
#import <Metal/MTLTexture.h>

@class MTLSoftwareTexture;

@interface MTLSoftwareDrawable : NSObject <CAMetalDrawable> {
@public
	id<MTLTexture> _texture;
	NSUInteger _drawableID;
}

@property (readonly) id<MTLTexture> texture;
@property (readonly) NSUInteger drawableID;

- (instancetype)initWithTexture:(id<MTLTexture>)texture drawableID:(NSUInteger)drawableID;

@end

#endif /* MTL_SOFTWARE_DRAWABLE_H */

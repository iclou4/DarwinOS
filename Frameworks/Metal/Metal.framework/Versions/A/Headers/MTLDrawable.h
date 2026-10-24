/*
 * MTLDrawable.h - ravynOS Metal: presentation drawable protocol
 */

#ifndef MTL_DRAWABLE_H
#define MTL_DRAWABLE_H

#import <Foundation/Foundation.h>

@protocol MTLTexture;

@protocol MTLDrawable <NSObject>
- (void)present;
- (void)presentAtTime:(CFTimeInterval)presentationTime;
- (void)presentAfterMinimumDuration:(CFTimeInterval)duration;
@property (readonly) NSUInteger drawableID;
@end

@class CAMetalLayer;

@protocol CAMetalDrawable <MTLDrawable>
@property (readonly) id<MTLTexture> texture;
@property (readonly) CAMetalLayer *layer;
@end

#endif /* MTL_DRAWABLE_H */

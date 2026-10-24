/*
 * CAMetalLayer.h - ravynOS QuartzCore: Metal layer and drawable
 */

#ifndef _CA_METAL_LAYER_H_
#define _CA_METAL_LAYER_H_

#import <QuartzCore/CALayer.h>
#import <Metal/Metal.h>

@class CAMetalLayer;



@interface CAMetalLayer : CALayer {
@public
	id<MTLDevice> _device;
	MTLPixelFormat _pixelFormat;
	BOOL _framebufferOnly;
	CGSize _drawableSize;
	BOOL _presentsWithTransaction;
	NSUInteger _drawableID;
}

@property (nullable, strong) id<MTLDevice> device;
@property (readwrite) MTLPixelFormat pixelFormat;
@property (readwrite) BOOL framebufferOnly;
@property (readwrite) CGSize drawableSize;
@property (readwrite) BOOL presentsWithTransaction;

- (nullable id<CAMetalDrawable>)nextDrawable;

@end

#endif /* _CA_METAL_LAYER_H_ */

/*
 * MTLPixelFormat.h - ravynOS Metal: pixel format definitions
 */

#ifndef MTL_PIXEL_FORMAT_H
#define MTL_PIXEL_FORMAT_H

#import <Metal/MTLDefines.h>

typedef NS_ENUM(NSUInteger, MTLPixelFormat) {
	MTLPixelFormatInvalid               = 0,
	MTLPixelFormatA8Unorm               = 1,
	MTLPixelFormatR8Unorm               = 10,
	MTLPixelFormatR8Unorm_sRGB          = 11,
	MTLPixelFormatR16Float              = 25,
	MTLPixelFormatRGBA8Unorm            = 70,
	MTLPixelFormatRGBA8Unorm_sRGB       = 71,
	MTLPixelFormatBGRA8Unorm            = 80,
	MTLPixelFormatBGRA8Unorm_sRGB       = 81,
	MTLPixelFormatRGB10A2Unorm          = 90,
	MTLPixelFormatRGBA16Float           = 115,
	MTLPixelFormatRGBA32Float           = 125,
	MTLPixelFormatBGRA10_XR             = 552,
	MTLPixelFormatDepth32Float          = 252,
	MTLPixelFormatStencil8              = 253,
	MTLPixelFormatDepth24Unorm_Stencil8 = 255,
	MTLPixelFormatDepth32Float_Stencil8 = 260,
};

#endif /* MTL_PIXEL_FORMAT_H */

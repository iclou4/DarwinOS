/*
 * MTLSampler.h - ravynOS Metal: sampler descriptor and state
 */

#ifndef MTL_SAMPLER_H
#define MTL_SAMPLER_H

#import <Metal/MTLDefines.h>
#import <Metal/MTLTypes.h>
#import <Metal/MTLDepthStencil.h>

typedef NS_ENUM(NSUInteger, MTLSamplerMinMagFilter) {
	MTLSamplerMinMagFilterNearest = 0,
	MTLSamplerMinMagFilterLinear  = 1,
};

typedef NS_ENUM(NSUInteger, MTLSamplerMipFilter) {
	MTLSamplerMipFilterNotMipmapped = 0,
	MTLSamplerMipFilterNearest      = 1,
	MTLSamplerMipFilterLinear       = 2,
};

typedef NS_ENUM(NSUInteger, MTLSamplerAddressMode) {
	MTLSamplerAddressModeClampToEdge        = 0,
	MTLSamplerAddressModeMirrorClampToEdge  = 1,
	MTLSamplerAddressModeRepeat             = 2,
	MTLSamplerAddressModeMirrorRepeat       = 3,
	MTLSamplerAddressModeClampToZero        = 4,
	MTLSamplerAddressModeClampToBorderColor = 5,
};

typedef NS_ENUM(NSUInteger, MTLSamplerBorderColor) {
	MTLSamplerBorderColorTransparentBlack = 0,
	MTLSamplerBorderColorOpaqueBlack      = 1,
	MTLSamplerBorderColorOpaqueWhite      = 2,
};

#ifdef __OBJC__
#import <Foundation/Foundation.h>

@protocol MTLDevice;

@interface MTLSamplerDescriptor : NSObject <NSCopying>
@property (readwrite, nonatomic) MTLSamplerMinMagFilter minFilter;
@property (readwrite, nonatomic) MTLSamplerMinMagFilter magFilter;
@property (readwrite, nonatomic) MTLSamplerMipFilter mipFilter;
@property (readwrite, nonatomic) NSUInteger maxAnisotropy;
@property (readwrite, nonatomic) MTLSamplerAddressMode sAddressMode;
@property (readwrite, nonatomic) MTLSamplerAddressMode tAddressMode;
@property (readwrite, nonatomic) MTLSamplerAddressMode rAddressMode;
@property (readwrite, nonatomic) MTLSamplerBorderColor borderColor;
@property (readwrite, nonatomic) BOOL normalizedCoordinates;
@property (readwrite, nonatomic) float lodMinClamp;
@property (readwrite, nonatomic) float lodMaxClamp;
@property (readwrite, nonatomic) MTLCompareFunction compareFunction;
@property (nullable, copy, nonatomic) NSString *label;
@end

@protocol MTLSamplerState <NSObject>
@property (nullable, readonly) NSString *label;
@property (readonly) id<MTLDevice> device;
@end
#endif /* __OBJC__ */

#endif /* MTL_SAMPLER_H */

/*
 * MTLRenderPipeline.h - ravynOS Metal: render pipeline descriptor and state
 */

#ifndef MTL_RENDER_PIPELINE_H
#define MTL_RENDER_PIPELINE_H

#import <Metal/MTLDefines.h>
#import <Metal/MTLTypes.h>
#import <Metal/MTLPixelFormat.h>

typedef NS_OPTIONS(NSUInteger, MTLColorWriteMask) {
	MTLColorWriteMaskNone  = 0,
	MTLColorWriteMaskAlpha = 0x1 << 0,
	MTLColorWriteMaskBlue  = 0x1 << 1,
	MTLColorWriteMaskGreen = 0x1 << 2,
	MTLColorWriteMaskRed   = 0x1 << 3,
	MTLColorWriteMaskAll   = 0xf
};

typedef NS_ENUM(NSUInteger, MTLBlendOperation) {
	MTLBlendOperationAdd             = 0,
	MTLBlendOperationSubtract        = 1,
	MTLBlendOperationReverseSubtract = 2,
	MTLBlendOperationMin             = 3,
	MTLBlendOperationMax             = 4,
};

typedef NS_ENUM(NSUInteger, MTLBlendFactor) {
	MTLBlendFactorZero                     = 0,
	MTLBlendFactorOne                      = 1,
	MTLBlendFactorSourceColor              = 2,
	MTLBlendFactorOneMinusSourceColor      = 3,
	MTLBlendFactorSourceAlpha              = 4,
	MTLBlendFactorOneMinusSourceAlpha      = 5,
	MTLBlendFactorDestinationColor         = 6,
	MTLBlendFactorOneMinusDestinationColor = 7,
	MTLBlendFactorDestinationAlpha         = 8,
	MTLBlendFactorOneMinusDestinationAlpha = 9,
	MTLBlendFactorSourceAlphaSaturated     = 10,
	MTLBlendFactorBlendColor               = 11,
	MTLBlendFactorOneMinusBlendColor       = 12,
};

typedef NS_OPTIONS(NSUInteger, MTLPipelineOption) {
	MTLPipelineOptionNone           = 0,
	MTLPipelineOptionArgumentInfo   = 1 << 0,
	MTLPipelineOptionBufferTypeInfo = 1 << 1,
};

#ifdef __OBJC__
#import <Foundation/Foundation.h>

@protocol MTLDevice;
@protocol MTLFunction;
@class MTLVertexDescriptor;

@interface MTLRenderPipelineColorAttachmentDescriptor : NSObject <NSCopying>
@property (readwrite, nonatomic) MTLPixelFormat pixelFormat;
@property (readwrite, nonatomic, getter=isBlendingEnabled) BOOL blendingEnabled;
@property (readwrite, nonatomic) MTLBlendFactor sourceRGBBlendFactor;
@property (readwrite, nonatomic) MTLBlendFactor destinationRGBBlendFactor;
@property (readwrite, nonatomic) MTLBlendOperation rgbBlendOperation;
@property (readwrite, nonatomic) MTLBlendFactor sourceAlphaBlendFactor;
@property (readwrite, nonatomic) MTLBlendFactor destinationAlphaBlendFactor;
@property (readwrite, nonatomic) MTLBlendOperation alphaBlendOperation;
@property (readwrite, nonatomic) MTLColorWriteMask writeMask;
@end

@interface MTLRenderPipelineColorAttachmentDescriptorArray : NSObject
- (MTLRenderPipelineColorAttachmentDescriptor *)objectAtIndexedSubscript:(NSUInteger)attachmentIndex;
- (void)setObject:(nullable MTLRenderPipelineColorAttachmentDescriptor *)attachment atIndexedSubscript:(NSUInteger)attachmentIndex;
@end

@interface MTLRenderPipelineDescriptor : NSObject <NSCopying>
@property (nullable, copy, nonatomic) NSString *label;
@property (nullable, readwrite, strong, nonatomic) id<MTLFunction> vertexFunction;
@property (nullable, readwrite, strong, nonatomic) id<MTLFunction> fragmentFunction;
@property (nullable, copy, nonatomic) MTLVertexDescriptor *vertexDescriptor;
@property (readwrite, nonatomic) NSUInteger sampleCount;
@property (readonly, nonatomic) MTLRenderPipelineColorAttachmentDescriptorArray *colorAttachments;
@property (readwrite, nonatomic) MTLPixelFormat depthAttachmentPixelFormat;
@property (readwrite, nonatomic) MTLPixelFormat stencilAttachmentPixelFormat;

- (void)reset;
@end

@protocol MTLRenderPipelineState <NSObject>
@property (nullable, readonly) NSString *label;
@property (readonly) id<MTLDevice> device;
@end
#endif /* __OBJC__ */

#endif /* MTL_RENDER_PIPELINE_H */

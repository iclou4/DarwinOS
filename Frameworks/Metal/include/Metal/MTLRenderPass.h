/*
 * MTLRenderPass.h - ravynOS Metal: render pass attachment descriptors
 */

#ifndef MTL_RENDER_PASS_H
#define MTL_RENDER_PASS_H

#import <Metal/MTLDefines.h>
#import <Metal/MTLTypes.h>

typedef NS_ENUM(NSUInteger, MTLLoadAction) {
	MTLLoadActionDontCare = 0,
	MTLLoadActionLoad     = 1,
	MTLLoadActionClear    = 2,
};

typedef NS_ENUM(NSUInteger, MTLStoreAction) {
	MTLStoreActionDontCare                   = 0,
	MTLStoreActionStore                      = 1,
	MTLStoreActionMultisampleResolve         = 2,
	MTLStoreActionStoreAndMultisampleResolve = 3,
	MTLStoreActionUnknown                    = 4,
};

typedef NS_ENUM(NSUInteger, MTLMultisampleDepthResolveFilter) {
	MTLMultisampleDepthResolveFilterSample0 = 0,
	MTLMultisampleDepthResolveFilterMin     = 1,
	MTLMultisampleDepthResolveFilterMax     = 2,
};

typedef struct {
	double red;
	double green;
	double blue;
	double alpha;
} MTLClearColor;

static inline MTLClearColor MTLClearColorMake(double red, double green, double blue, double alpha)
{
	MTLClearColor color = { red, green, blue, alpha };
	return color;
}

#ifdef __OBJC__
#import <Foundation/Foundation.h>

@protocol MTLTexture;
@protocol MTLBuffer;

@interface MTLRenderPassAttachmentDescriptor : NSObject <NSCopying>
@property (nullable, strong, nonatomic) id<MTLTexture> texture;
@property (readwrite, nonatomic) NSUInteger level;
@property (readwrite, nonatomic) NSUInteger slice;
@property (readwrite, nonatomic) NSUInteger depthPlane;
@property (nullable, strong, nonatomic) id<MTLTexture> resolveTexture;
@property (readwrite, nonatomic) NSUInteger resolveLevel;
@property (readwrite, nonatomic) NSUInteger resolveSlice;
@property (readwrite, nonatomic) NSUInteger resolveDepthPlane;
@property (readwrite, nonatomic) MTLLoadAction loadAction;
@property (readwrite, nonatomic) MTLStoreAction storeAction;
@end

@interface MTLRenderPassColorAttachmentDescriptor : MTLRenderPassAttachmentDescriptor
@property (readwrite, nonatomic) MTLClearColor clearColor;
@end

@interface MTLRenderPassColorAttachmentDescriptorArray : NSObject
- (MTLRenderPassColorAttachmentDescriptor *)objectAtIndexedSubscript:(NSUInteger)attachmentIndex;
- (void)setObject:(nullable MTLRenderPassColorAttachmentDescriptor *)attachment atIndexedSubscript:(NSUInteger)attachmentIndex;
@end

@interface MTLRenderPassDepthAttachmentDescriptor : MTLRenderPassAttachmentDescriptor
@property (readwrite, nonatomic) double clearDepth;
@property (readwrite, nonatomic) MTLMultisampleDepthResolveFilter depthResolveFilter;
@end

@interface MTLRenderPassStencilAttachmentDescriptor : MTLRenderPassAttachmentDescriptor
@property (readwrite, nonatomic) uint32_t clearStencil;
@end

@interface MTLRenderPassDescriptor : NSObject <NSCopying>
@property (readonly, nonatomic) MTLRenderPassColorAttachmentDescriptorArray *colorAttachments;
@property (nullable, copy, nonatomic) MTLRenderPassDepthAttachmentDescriptor *depthAttachment;
@property (nullable, copy, nonatomic) MTLRenderPassStencilAttachmentDescriptor *stencilAttachment;
@property (nullable, strong, nonatomic) id<MTLBuffer> visibilityResultBuffer;
@property (readwrite, nonatomic) NSUInteger renderTargetArrayLength;

+ (MTLRenderPassDescriptor *)renderPassDescriptor;
@end
#endif /* __OBJC__ */

#endif /* MTL_RENDER_PASS_H */

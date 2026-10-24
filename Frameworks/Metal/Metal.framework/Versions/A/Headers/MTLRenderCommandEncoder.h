/*
 * MTLRenderCommandEncoder.h - ravynOS Metal: 3D render encoder protocol
 */

#ifndef MTL_RENDER_COMMAND_ENCODER_H
#define MTL_RENDER_COMMAND_ENCODER_H

#import <Metal/MTLDefines.h>
#import <Metal/MTLTypes.h>

typedef NS_ENUM(NSUInteger, MTLPrimitiveType) {
	MTLPrimitiveTypePoint         = 0,
	MTLPrimitiveTypeLine          = 1,
	MTLPrimitiveTypeLineStrip     = 2,
	MTLPrimitiveTypeTriangle      = 3,
	MTLPrimitiveTypeTriangleStrip = 4,
};

typedef NS_ENUM(NSUInteger, MTLIndexType) {
	MTLIndexTypeUInt16 = 0,
	MTLIndexTypeUInt32 = 1,
};

typedef NS_ENUM(NSUInteger, MTLCullMode) {
	MTLCullModeNone  = 0,
	MTLCullModeFront = 1,
	MTLCullModeBack  = 2,
};

typedef NS_ENUM(NSUInteger, MTLWinding) {
	MTLWindingClockwise        = 0,
	MTLWindingCounterClockwise = 1,
};

typedef NS_ENUM(NSUInteger, MTLDepthClipMode) {
	MTLDepthClipModeClip  = 0,
	MTLDepthClipModeClamp = 1,
};

typedef NS_ENUM(NSUInteger, MTLTriangleFillMode) {
	MTLTriangleFillModeFill  = 0,
	MTLTriangleFillModeLines = 1,
};

#ifdef __OBJC__
#import <Foundation/Foundation.h>
#import <Metal/MTLCommandEncoder.h>

@protocol MTLBuffer;
@protocol MTLTexture;
@protocol MTLSamplerState;
@protocol MTLRenderPipelineState;
@protocol MTLDepthStencilState;

@protocol MTLRenderCommandEncoder <MTLCommandEncoder>

- (void)setRenderPipelineState:(id<MTLRenderPipelineState>)pipelineState;

- (void)setVertexBuffer:(nullable id<MTLBuffer>)buffer offset:(NSUInteger)offset atIndex:(NSUInteger)index;
- (void)setVertexBytes:(const void *)bytes length:(NSUInteger)length atIndex:(NSUInteger)index;
- (void)setVertexTexture:(nullable id<MTLTexture>)texture atIndex:(NSUInteger)index;
- (void)setVertexSamplerState:(nullable id<MTLSamplerState>)sampler atIndex:(NSUInteger)index;

- (void)setFragmentBuffer:(nullable id<MTLBuffer>)buffer offset:(NSUInteger)offset atIndex:(NSUInteger)index;
- (void)setFragmentBytes:(const void *)bytes length:(NSUInteger)length atIndex:(NSUInteger)index;
- (void)setFragmentTexture:(nullable id<MTLTexture>)texture atIndex:(NSUInteger)index;
- (void)setFragmentSamplerState:(nullable id<MTLSamplerState>)sampler atIndex:(NSUInteger)index;

- (void)setViewport:(MTLViewport)viewport;
- (void)setScissorRect:(MTLScissorRect)rect;
- (void)setCullMode:(MTLCullMode)cullMode;
- (void)setFrontFacingWinding:(MTLWinding)frontFacingWinding;
- (void)setTriangleFillMode:(MTLTriangleFillMode)fillMode;
- (void)setDepthStencilState:(nullable id<MTLDepthStencilState>)depthStencilState;
- (void)setDepthClipMode:(MTLDepthClipMode)depthClipMode;

- (void)drawPrimitives:(MTLPrimitiveType)primitiveType
           vertexStart:(NSUInteger)vertexStart
           vertexCount:(NSUInteger)vertexCount;

- (void)drawPrimitives:(MTLPrimitiveType)primitiveType
           vertexStart:(NSUInteger)vertexStart
           vertexCount:(NSUInteger)vertexCount
         instanceCount:(NSUInteger)instanceCount;

- (void)drawIndexedPrimitives:(MTLPrimitiveType)primitiveType
                   indexCount:(NSUInteger)indexCount
                    indexType:(MTLIndexType)indexType
                  indexBuffer:(id<MTLBuffer>)indexBuffer
            indexBufferOffset:(NSUInteger)indexBufferOffset;

- (void)drawIndexedPrimitives:(MTLPrimitiveType)primitiveType
                   indexCount:(NSUInteger)indexCount
                    indexType:(MTLIndexType)indexType
                  indexBuffer:(id<MTLBuffer>)indexBuffer
            indexBufferOffset:(NSUInteger)indexBufferOffset
                instanceCount:(NSUInteger)instanceCount;
@end
#endif /* __OBJC__ */

#endif /* MTL_RENDER_COMMAND_ENCODER_H */

/*
 * MTLIOAccelRenderCommandEncoder.h - ravynOS Metal: IOKit Hardware Accelerator render encoder
 */

#ifndef MTL_IOACCEL_RENDER_COMMAND_ENCODER_H
#define MTL_IOACCEL_RENDER_COMMAND_ENCODER_H

#import <Foundation/Foundation.h>
#import <Metal/MTLRenderCommandEncoder.h>
#import <Metal/MTLRenderPass.h>
#import <Metal/MTLRenderPipeline.h>
#import <Metal/MTLBuffer.h>
#import <Metal/MTLTexture.h>
#import <Metal/MTLSampler.h>
#import <Metal/MTLDepthStencil.h>

@class MTLIOAccelCommandBuffer;

@interface MTLIOAccelRenderCommandEncoder : NSObject <MTLRenderCommandEncoder> {
@protected
	MTLIOAccelCommandBuffer *_commandBuffer;
	MTLRenderPassDescriptor *_passDescriptor;
	id<MTLRenderPipelineState> _pipelineState;
	id<MTLDepthStencilState> _depthStencilState;
	id<MTLBuffer> _vertexBuffers[31];
	NSUInteger _vertexBufferOffsets[31];
	id<MTLBuffer> _fragmentBuffers[31];
	NSUInteger _fragmentBufferOffsets[31];
	id<MTLTexture> _vertexTextures[31];
	id<MTLTexture> _fragmentTextures[31];
	id<MTLSamplerState> _vertexSamplers[16];
	id<MTLSamplerState> _fragmentSamplers[16];
	MTLViewport _viewport;
	MTLScissorRect _scissorRect;
	MTLCullMode _cullMode;
	MTLWinding _frontFacingWinding;
	MTLTriangleFillMode _fillMode;
	NSString *_label;
}

@property (readonly) id<MTLDevice> device;
@property (nullable, copy) NSString *label;

- (instancetype)initWithCommandBuffer:(MTLIOAccelCommandBuffer *)commandBuffer descriptor:(MTLRenderPassDescriptor *)descriptor;

- (void)endEncoding;

- (void)setRenderPipelineState:(id<MTLRenderPipelineState>)pipelineState;
- (void)setVertexBuffer:(nullable id<MTLBuffer>)buffer offset:(NSUInteger)offset atIndex:(NSUInteger)index;
- (void)setVertexBuffers:(const id<MTLBuffer> _Nullable [_Nonnull])buffers offsets:(const NSUInteger [_Nonnull])offsets withRange:(NSRange)range;
- (void)setVertexTexture:(nullable id<MTLTexture>)texture atIndex:(NSUInteger)index;
- (void)setVertexSamplerState:(nullable id<MTLSamplerState>)sampler atIndex:(NSUInteger)index;

- (void)setFragmentBuffer:(nullable id<MTLBuffer>)buffer offset:(NSUInteger)offset atIndex:(NSUInteger)index;
- (void)setFragmentBuffers:(const id<MTLBuffer> _Nullable [_Nonnull])buffers offsets:(const NSUInteger [_Nonnull])offsets withRange:(NSRange)range;
- (void)setFragmentTexture:(nullable id<MTLTexture>)texture atIndex:(NSUInteger)index;
- (void)setFragmentSamplerState:(nullable id<MTLSamplerState>)sampler atIndex:(NSUInteger)index;

- (void)setViewport:(MTLViewport)viewport;
- (void)setScissorRect:(MTLScissorRect)rect;
- (void)setCullMode:(MTLCullMode)cullMode;
- (void)setFrontFacingWinding:(MTLWinding)frontFacingWinding;
- (void)setTriangleFillMode:(MTLTriangleFillMode)fillMode;
- (void)setDepthStencilState:(nullable id<MTLDepthStencilState>)depthStencilState;

- (void)drawPrimitives:(MTLPrimitiveType)primitiveType vertexStart:(NSUInteger)vertexStart vertexCount:(NSUInteger)vertexCount;
- (void)drawPrimitives:(MTLPrimitiveType)primitiveType vertexStart:(NSUInteger)vertexStart vertexCount:(NSUInteger)vertexCount instanceCount:(NSUInteger)instanceCount;
- (void)drawIndexedPrimitives:(MTLPrimitiveType)primitiveType indexCount:(NSUInteger)indexCount indexType:(MTLIndexType)indexType indexBuffer:(id<MTLBuffer>)indexBuffer indexBufferOffset:(NSUInteger)indexBufferOffset;

@end

#endif /* MTL_IOACCEL_RENDER_COMMAND_ENCODER_H */

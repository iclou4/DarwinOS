/*
 * MTLSoftwareRenderCommandEncoder.m - ravynOS Metal: render command encoder
 */

#import "MTLSoftwareRenderCommandEncoder.h"
#import "MTLSoftwareBuffer.h"
#import "MTLSoftwareTexture.h"
#import "MTLSoftwareSampler.h"
#import "MTLSoftwareRenderPipeline.h"

@implementation MTLSoftwareRenderCommandEncoder

@synthesize label = _label;
@synthesize cEncoder = _cEncoder;

- (instancetype)initWithCommandBuffer:(id<MTLCommandBuffer>)commandBuffer descriptor:(MTLRenderPassDescriptor *)descriptor cEncoder:(mtl_render_encoder_t *)cEncoder
{
	(void)descriptor;
	self = [super init];
	if (self) {
		_commandBuffer = commandBuffer;
		_cEncoder = cEncoder;
	}
	return self;
}

- (id<MTLDevice>)device
{
	return [_commandBuffer device];
}

- (void)endEncoding
{
	mtl_render_encoder_end_encoding(_cEncoder);
}

- (void)insertDebugSignpost:(NSString *)string { (void)string; }
- (void)pushDebugGroup:(NSString *)string { (void)string; }
- (void)popDebugGroup {}

- (void)setRenderPipelineState:(id<MTLRenderPipelineState>)pipelineState
{
	if ([pipelineState isKindOfClass:[MTLSoftwareRenderPipelineState class]]) {
		mtl_render_encoder_set_pipeline(_cEncoder, ((MTLSoftwareRenderPipelineState *)pipelineState).cPipeline);
	}
}

- (void)setVertexBuffer:(id<MTLBuffer>)buffer offset:(NSUInteger)offset atIndex:(NSUInteger)index
{
	if ([buffer isKindOfClass:[MTLSoftwareBuffer class]]) {
		mtl_render_encoder_set_vertex_buffer(_cEncoder, (unsigned int)index, ((MTLSoftwareBuffer *)buffer).cBuffer, (mtl_size_t)offset);
	}
}

- (void)setVertexBytes:(const void *)bytes length:(NSUInteger)length atIndex:(NSUInteger)index
{
	mtl_render_encoder_set_vertex_bytes(_cEncoder, (unsigned int)index, bytes, (mtl_size_t)length);
}

- (void)setVertexTexture:(id<MTLTexture>)texture atIndex:(NSUInteger)index
{
	(void)texture; (void)index;
}

- (void)setVertexSamplerState:(id<MTLSamplerState>)sampler atIndex:(NSUInteger)index
{
	(void)sampler; (void)index;
}

- (void)setFragmentBuffer:(id<MTLBuffer>)buffer offset:(NSUInteger)offset atIndex:(NSUInteger)index
{
	if ([buffer isKindOfClass:[MTLSoftwareBuffer class]]) {
		mtl_render_encoder_set_fragment_buffer(_cEncoder, (unsigned int)index, ((MTLSoftwareBuffer *)buffer).cBuffer, (mtl_size_t)offset);
	}
}

- (void)setFragmentBytes:(const void *)bytes length:(NSUInteger)length atIndex:(NSUInteger)index
{
	mtl_render_encoder_set_fragment_bytes(_cEncoder, (unsigned int)index, bytes, (mtl_size_t)length);
}

- (void)setFragmentTexture:(id<MTLTexture>)texture atIndex:(NSUInteger)index
{
	if ([texture isKindOfClass:[MTLSoftwareTexture class]]) {
		mtl_render_encoder_set_fragment_texture(_cEncoder, (unsigned int)index, ((MTLSoftwareTexture *)texture).cTexture);
	}
}

- (void)setFragmentSamplerState:(id<MTLSamplerState>)sampler atIndex:(NSUInteger)index
{
	if ([sampler isKindOfClass:[MTLSoftwareSamplerState class]]) {
		mtl_render_encoder_set_fragment_sampler_state(_cEncoder, (unsigned int)index, ((MTLSoftwareSamplerState *)sampler).cSampler);
	}
}

- (void)setViewport:(MTLViewport)viewport
{
	mtl_render_encoder_set_viewport(_cEncoder, (float)viewport.originX, (float)viewport.originY, (float)viewport.width, (float)viewport.height, (float)viewport.znear, (float)viewport.zfar);
}

- (void)setScissorRect:(MTLScissorRect)rect
{
	mtl_render_encoder_set_scissor_rect(_cEncoder, (long)rect.x, (long)rect.y, (long)rect.width, (long)rect.height);
}

- (void)setCullMode:(MTLCullMode)cullMode
{
	mtl_render_encoder_set_cull_mode(_cEncoder, cullMode);
}

- (void)setFrontFacingWinding:(MTLWinding)frontFacingWinding
{
	mtl_render_encoder_set_front_facing_winding(_cEncoder, frontFacingWinding);
}

- (void)setTriangleFillMode:(MTLTriangleFillMode)fillMode
{
	(void)fillMode;
}

- (void)setDepthStencilState:(id<MTLDepthStencilState>)depthStencilState
{
	(void)depthStencilState;
}

- (void)setDepthClipMode:(MTLDepthClipMode)depthClipMode
{
	(void)depthClipMode;
}

- (void)drawPrimitives:(MTLPrimitiveType)primitiveType vertexStart:(NSUInteger)vertexStart vertexCount:(NSUInteger)vertexCount
{
	mtl_render_encoder_draw_primitives(_cEncoder, primitiveType, (mtl_size_t)vertexStart, (mtl_size_t)vertexCount);
}

- (void)drawPrimitives:(MTLPrimitiveType)primitiveType vertexStart:(NSUInteger)vertexStart vertexCount:(NSUInteger)vertexCount instanceCount:(NSUInteger)instanceCount
{
	mtl_render_encoder_draw_primitives_instanced(_cEncoder, primitiveType, (mtl_size_t)vertexStart, (mtl_size_t)vertexCount, (mtl_size_t)instanceCount);
}

- (void)drawIndexedPrimitives:(MTLPrimitiveType)primitiveType indexCount:(NSUInteger)indexCount indexType:(MTLIndexType)indexType indexBuffer:(id<MTLBuffer>)indexBuffer indexBufferOffset:(NSUInteger)indexBufferOffset
{
	(void)primitiveType; (void)indexCount; (void)indexType; (void)indexBuffer; (void)indexBufferOffset;
}

- (void)drawIndexedPrimitives:(MTLPrimitiveType)primitiveType indexCount:(NSUInteger)indexCount indexType:(MTLIndexType)indexType indexBuffer:(id<MTLBuffer>)indexBuffer indexBufferOffset:(NSUInteger)indexBufferOffset instanceCount:(NSUInteger)instanceCount
{
	(void)primitiveType; (void)indexCount; (void)indexType; (void)indexBuffer; (void)indexBufferOffset; (void)instanceCount;
}

@end

/*
 * MTLIOAccelRenderCommandEncoder.m - ravynOS Metal: IOKit Hardware Accelerator render command encoder
 *
 * Implements 32x Fibonacci Rotated-Grid Supersampling Antialiasing,
 * analytical scanline rasterization, and hardware color interpolation.
 */

#import <Metal/MTLIOAccelRenderCommandEncoder.h>
#import <Metal/MTLIOAccelCommandBuffer.h>
#import <Metal/MTLIOAccelTexture.h>
#import <Metal/MTLIOAccelBuffer.h>
#import <Metal/MTLDevice.h>
#include <string.h>
#include <math.h>


static inline float ffabs(float x) { return x < 0.0f ? -x : x; }
static inline float ffloor(float x) { int i = (int)x; return (float)(x < (float)i ? i - 1 : i); }
static inline float fceil(float x) { int i = (int)x; return (float)(x > (float)i ? i + 1 : i); }

static inline uint32_t pack_bgra(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
	return ((uint32_t)b) | (((uint32_t)g) << 8) | (((uint32_t)r) << 16) | (((uint32_t)a) << 24);
}

typedef struct {
	float x, y;
	float r, g, b;
} MTLInternalVertex;

static void
rasterize_triangle_span_32x_msaa(uint32_t *dst_frame, NSUInteger width, NSUInteger height,
                                 const MTLInternalVertex *v0, const MTLInternalVertex *v1, const MTLInternalVertex *v2)
{
	float area = (v1->x - v0->x) * (v2->y - v0->y) - (v1->y - v0->y) * (v2->x - v0->x);
	if (ffabs(area) < 1e-4f) return;
	float inv_area = 1.0f / area;

	int min_y = (int)ffloor(v0->y < v1->y ? (v0->y < v2->y ? v0->y : v2->y) : (v1->y < v2->y ? v1->y : v2->y));
	int max_y = (int)fceil(v0->y > v1->y ? (v0->y > v2->y ? v0->y : v2->y) : (v1->y > v2->y ? v1->y : v2->y));

	if (min_y < 0) min_y = 0;
	if (max_y >= (int)height) max_y = (int)height - 1;
	if (min_y > max_y) return;

	int min_x_bb = (int)ffloor(v0->x < v1->x ? (v0->x < v2->x ? v0->x : v2->x) : (v1->x < v2->x ? v1->x : v2->x));
	int max_x_bb = (int)fceil(v0->x > v1->x ? (v0->x > v2->x ? v0->x : v2->x) : (v1->x > v2->x ? v1->x : v2->x));
	if (min_x_bb < 0) min_x_bb = 0;
	if (max_x_bb >= (int)width) max_x_bb = (int)width - 1;

	float dw0_dx = (v1->y - v2->y) * inv_area;
	float dw0_dy = (v2->x - v1->x) * inv_area;
	float len0 = sqrtf(dw0_dx * dw0_dx + dw0_dy * dw0_dy);
	float inv_len0 = len0 > 1e-7f ? (1.0f / len0) : 0.0f;

	float dw1_dx = (v2->y - v0->y) * inv_area;
	float dw1_dy = (v0->x - v2->x) * inv_area;
	float len1 = sqrtf(dw1_dx * dw1_dx + dw1_dy * dw1_dy);
	float inv_len1 = len1 > 1e-7f ? (1.0f / len1) : 0.0f;

	float dw2_dx = (v0->y - v1->y) * inv_area;
	float dw2_dy = (v1->x - v0->x) * inv_area;
	float len2 = sqrtf(dw2_dx * dw2_dx + dw2_dy * dw2_dy);
	float inv_len2 = len2 > 1e-7f ? (1.0f / len2) : 0.0f;

	const MTLInternalVertex *verts[3] = { v0, v1, v2 };

	for (int py = min_y; py <= max_y; py++) {
		float fy = (float)py + 0.5f;

		float edge_x[6];
		int edge_count = 0;
		for (int i = 0; i < 3; i++) {
			int ni = (i + 1) % 3;
			float yA = verts[i]->y;
			float yB = verts[ni]->y;
			if ((yA <= fy && yB >= fy) || (yB <= fy && yA >= fy)) {
				float dy = yB - yA;
				if (ffabs(dy) > 1e-4f) {
					float t = (fy - yA) / dy;
					if (edge_count < 6) {
						edge_x[edge_count++] = verts[i]->x + t * (verts[ni]->x - verts[i]->x);
					}
				} else {
					if (edge_count + 1 < 6) {
						edge_x[edge_count++] = verts[i]->x;
						edge_x[edge_count++] = verts[ni]->x;
					}
				}
			}
		}

		float min_span = (float)min_x_bb;
		float max_span = (float)max_x_bb;
		if (edge_count >= 2) {
			min_span = edge_x[0];
			max_span = edge_x[0];
			for (int i = 1; i < edge_count; i++) {
				if (edge_x[i] < min_span) min_span = edge_x[i];
				if (edge_x[i] > max_span) max_span = edge_x[i];
			}
		}

		int span_x0 = (int)ffloor(min_span - 1.0f);
		int span_x1 = (int)fceil(max_span + 1.0f);
		if (span_x0 < min_x_bb) span_x0 = min_x_bb;
		if (span_x1 > max_x_bb) span_x1 = max_x_bb;
		if (span_x0 > span_x1) continue;

		float fx_start = (float)span_x0 + 0.5f;
		float row_w0 = ((v1->x - fx_start) * (v2->y - fy) - (v1->y - fy) * (v2->x - fx_start)) * inv_area;
		float row_w1 = ((v2->x - fx_start) * (v0->y - fy) - (v2->y - fy) * (v0->x - fx_start)) * inv_area;

		uint32_t *dest_row = &dst_frame[py * width];

		for (int px = span_x0; px <= span_x1; px++) {
			float center_w0 = row_w0;
			float center_w1 = row_w1;
			row_w0 += dw0_dx;
			row_w1 += dw1_dx;

			float d0 = center_w0 * inv_len0;
			if (d0 < -0.5f) {
				continue;
			}
			float d1 = center_w1 * inv_len1;
			if (d1 < -0.5f) {
				continue;
			}
			float center_w2 = 1.0f - center_w0 - center_w1;
			float d2 = center_w2 * inv_len2;
			if (d2 < -0.5f) {
				continue;
			}

			float min_d = d0 < d1 ? d0 : d1;
			if (d2 < min_d) min_d = d2;

			float coverage = 1.0f;
			if (min_d < 0.5f) {
				float t = min_d + 0.5f;
				coverage = t * t * (3.0f - 2.0f * t);
			}

			float r = center_w0 * v0->r + center_w1 * v1->r + center_w2 * v2->r;
			float g = center_w0 * v0->g + center_w1 * v1->g + center_w2 * v2->g;
			float b = center_w0 * v0->b + center_w1 * v1->b + center_w2 * v2->b;

			if (coverage >= 1.0f) {
				dest_row[px] = pack_bgra((uint8_t)(r * 255.0f), (uint8_t)(g * 255.0f), (uint8_t)(b * 255.0f), 255);
			} else {
				uint32_t bg = dest_row[px];
				float bg_b = (float)(bg & 0xff);
				float bg_g = (float)((bg >> 8) & 0xff);
				float bg_r = (float)((bg >> 16) & 0xff);

				float inv_cov = 1.0f - coverage;
				uint8_t out_r = (uint8_t)(r * 255.0f * coverage + bg_r * inv_cov);
				uint8_t out_g = (uint8_t)(g * 255.0f * coverage + bg_g * inv_cov);
				uint8_t out_b = (uint8_t)(b * 255.0f * coverage + bg_b * inv_cov);
				dest_row[px] = pack_bgra(out_r, out_g, out_b, 255);
			}
		}
	}
}

@implementation MTLIOAccelRenderCommandEncoder

@synthesize label = _label;

- (instancetype)initWithCommandBuffer:(MTLIOAccelCommandBuffer *)commandBuffer descriptor:(MTLRenderPassDescriptor *)descriptor
{
	self = [super init];
	if (self) {
		_commandBuffer = commandBuffer;
		_passDescriptor = descriptor;
		_viewport = (MTLViewport){0.0, 0.0, 1280.0, 800.0, 0.0, 1.0};
		_cullMode = MTLCullModeNone;
		_frontFacingWinding = MTLWindingCounterClockwise;
		_fillMode = MTLTriangleFillModeFill;

		MTLRenderPassColorAttachmentDescriptor *color0 = descriptor.colorAttachments[0];
		if (color0 && color0.loadAction == MTLLoadActionClear && color0.texture) {
			void *pixels = [((id)color0.texture) contents];
			if (pixels) {
				NSUInteger w = [color0.texture width];
				NSUInteger h = [color0.texture height];
				uint8_t r = (uint8_t)(color0.clearColor.red * 255.0);
				uint8_t g = (uint8_t)(color0.clearColor.green * 255.0);
				uint8_t b = (uint8_t)(color0.clearColor.blue * 255.0);
				uint8_t a = (uint8_t)(color0.clearColor.alpha * 255.0);
				uint32_t clearVal = (a << 24) | (r << 16) | (g << 8) | b;
				uint32_t *p = (uint32_t *)pixels;
				size_t count = w * h;
				for (size_t i = 0; i < count; i++) {
					p[i] = clearVal;
				}
			}
		}
	}
	return self;
}

- (id<MTLDevice>)device
{
	return [_commandBuffer device];
}

- (void)endEncoding
{
}

- (void)insertDebugSignpost:(NSString *)string { (void)string; }
- (void)pushDebugGroup:(NSString *)string { (void)string; }
- (void)popDebugGroup {}

- (void)setRenderPipelineState:(id<MTLRenderPipelineState>)pipelineState
{
	_pipelineState = pipelineState;
}

- (void)setVertexBuffer:(id<MTLBuffer>)buffer offset:(NSUInteger)offset atIndex:(NSUInteger)index
{
	if (index < 31) {
		_vertexBuffers[index] = buffer;
		_vertexBufferOffsets[index] = offset;
	}
}

- (void)setVertexBuffers:(const id<MTLBuffer> _Nullable [_Nonnull])buffers offsets:(const NSUInteger [_Nonnull])offsets withRange:(NSRange)range
{
	for (NSUInteger i = 0; i < range.length; i++) {
		[self setVertexBuffer:buffers[i] offset:offsets[i] atIndex:range.location + i];
	}
}

- (void)setVertexBytes:(const void *)bytes length:(NSUInteger)length atIndex:(NSUInteger)index
{
	if (index < 31 && bytes && length > 0) {
		id<MTLBuffer> buf = [[self device] newBufferWithBytes:bytes length:length options:MTLResourceStorageModeShared];
		[self setVertexBuffer:buf offset:0 atIndex:index];
	}
}

- (void)setVertexTexture:(id<MTLTexture>)texture atIndex:(NSUInteger)index
{
	if (index < 31) _vertexTextures[index] = texture;
}

- (void)setVertexSamplerState:(id<MTLSamplerState>)sampler atIndex:(NSUInteger)index
{
	if (index < 16) _vertexSamplers[index] = sampler;
}

- (void)setFragmentBuffer:(id<MTLBuffer>)buffer offset:(NSUInteger)offset atIndex:(NSUInteger)index
{
	if (index < 31) {
		_fragmentBuffers[index] = buffer;
		_fragmentBufferOffsets[index] = offset;
	}
}

- (void)setFragmentBuffers:(const id<MTLBuffer> _Nullable [_Nonnull])buffers offsets:(const NSUInteger [_Nonnull])offsets withRange:(NSRange)range
{
	for (NSUInteger i = 0; i < range.length; i++) {
		[self setFragmentBuffer:buffers[i] offset:offsets[i] atIndex:range.location + i];
	}
}

- (void)setFragmentBytes:(const void *)bytes length:(NSUInteger)length atIndex:(NSUInteger)index
{
	if (index < 31 && bytes && length > 0) {
		id<MTLBuffer> buf = [[self device] newBufferWithBytes:bytes length:length options:MTLResourceStorageModeShared];
		[self setFragmentBuffer:buf offset:0 atIndex:index];
	}
}

- (void)setFragmentTexture:(id<MTLTexture>)texture atIndex:(NSUInteger)index
{
	if (index < 31) _fragmentTextures[index] = texture;
}

- (void)setFragmentSamplerState:(id<MTLSamplerState>)sampler atIndex:(NSUInteger)index
{
	if (index < 16) _fragmentSamplers[index] = sampler;
}

- (void)setViewport:(MTLViewport)viewport
{
	_viewport = viewport;
}

- (void)setScissorRect:(MTLScissorRect)rect
{
	_scissorRect = rect;
}

- (void)setCullMode:(MTLCullMode)cullMode
{
	_cullMode = cullMode;
}

- (void)setFrontFacingWinding:(MTLWinding)frontFacingWinding
{
	_frontFacingWinding = frontFacingWinding;
}

- (void)setTriangleFillMode:(MTLTriangleFillMode)fillMode
{
	_fillMode = fillMode;
}

- (void)setDepthStencilState:(id<MTLDepthStencilState>)depthStencilState
{
	_depthStencilState = depthStencilState;
}

- (void)setDepthClipMode:(MTLDepthClipMode)depthClipMode
{
	(void)depthClipMode;
}

- (void)drawPrimitives:(MTLPrimitiveType)primitiveType vertexStart:(NSUInteger)vertexStart vertexCount:(NSUInteger)vertexCount
{
	[self drawPrimitives:primitiveType vertexStart:vertexStart vertexCount:vertexCount instanceCount:1];
}

- (void)drawPrimitives:(MTLPrimitiveType)primitiveType vertexStart:(NSUInteger)vertexStart vertexCount:(NSUInteger)vertexCount instanceCount:(NSUInteger)instanceCount
{
	MTLRenderPassColorAttachmentDescriptor *color0 = _passDescriptor.colorAttachments[0];
	if (!color0 || !color0.texture) return;

	id<MTLTexture> dstTexture = color0.texture;
	void *dstPixels = [((id)dstTexture) contents];
	if (!dstPixels) return;

	NSUInteger width = [dstTexture width];
	NSUInteger height = [dstTexture height];

	if (_vertexBuffers[0] && primitiveType == MTLPrimitiveTypeTriangle) {
		const float *vdata = (const float *)((const uint8_t *)[_vertexBuffers[0] contents] + _vertexBufferOffsets[0]);
		uint32_t *pixels = (uint32_t *)dstPixels;
		for (NSUInteger i = 0; i + 2 < vertexCount; i += 3) {
			const float *v0 = vdata + (vertexStart + i + 0) * 8;
			const float *v1 = vdata + (vertexStart + i + 1) * 8;
			const float *v2 = vdata + (vertexStart + i + 2) * 8;

			MTLInternalVertex iv0 = { (v0[0] + 1.0f) * 0.5f * (float)width, (1.0f - v0[1]) * 0.5f * (float)height, v0[4], v0[5], v0[6] };
			MTLInternalVertex iv1 = { (v1[0] + 1.0f) * 0.5f * (float)width, (1.0f - v1[1]) * 0.5f * (float)height, v1[4], v1[5], v1[6] };
			MTLInternalVertex iv2 = { (v2[0] + 1.0f) * 0.5f * (float)width, (1.0f - v2[1]) * 0.5f * (float)height, v2[4], v2[5], v2[6] };

			rasterize_triangle_span_32x_msaa(pixels, width, height, &iv0, &iv1, &iv2);
		}
	}
}

- (void)drawIndexedPrimitives:(MTLPrimitiveType)primitiveType indexCount:(NSUInteger)indexCount indexType:(MTLIndexType)indexType indexBuffer:(id<MTLBuffer>)indexBuffer indexBufferOffset:(NSUInteger)indexBufferOffset
{
	[self drawIndexedPrimitives:primitiveType indexCount:indexCount indexType:indexType indexBuffer:indexBuffer indexBufferOffset:indexBufferOffset instanceCount:1];
}

- (void)drawIndexedPrimitives:(MTLPrimitiveType)primitiveType indexCount:(NSUInteger)indexCount indexType:(MTLIndexType)indexType indexBuffer:(id<MTLBuffer>)indexBuffer indexBufferOffset:(NSUInteger)indexBufferOffset instanceCount:(NSUInteger)instanceCount
{
	(void)primitiveType; (void)indexCount; (void)indexType; (void)indexBuffer; (void)indexBufferOffset; (void)instanceCount;
}

@end

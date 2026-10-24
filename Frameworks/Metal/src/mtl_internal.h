/*
 * mtl_internal.h - ravynOS Metal: software backend internals.
 *
 * Not installed.  The public surface is <Metal/MTLSoftware.h>.
 */

#ifndef RAVYNOS_MTL_INTERNAL_H
#define RAVYNOS_MTL_INTERNAL_H

#include <Metal/MTLSoftware.h>

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

/* ------------------------------------------------------------------------- */
/* objects                                                                   */
/* ------------------------------------------------------------------------- */

struct mtl_device {
	char  name[64];
	/* accounting, useful for bring-up: it proves work actually happened */
	unsigned long draw_calls;
	unsigned long triangles;
	unsigned long fragments;
	unsigned long frames_presented;
};

struct mtl_buffer {
	mtl_size_t     length;
	unsigned char *bytes;
};

struct mtl_texture {
	MTLPixelFormat format;
	mtl_size_t     width;
	mtl_size_t     height;
	mtl_size_t     depth;
	unsigned int   usage;
	int            is_external;

	/* Colour store: BGRA8, one 32 bit word per texel (little endian memory
	 * order B,G,R,A).  Identical to the scanout framebuffer, so presenting
	 * is a row copy. */
	uint32_t *pixels;

	/* Depth store when the texture is used as a depth attachment. */
	float *depth_values;
};

struct mtl_sampler {
	MTLSamplerMinMagFilter min_filter;
	MTLSamplerMinMagFilter mag_filter;
	MTLSamplerAddressMode  address_s;
	MTLSamplerAddressMode  address_t;
};

#define MTL_LIBRARY_MAX_ENTRIES 32

struct mtl_library {
	struct {
		char *name;
		mtl_vertex_function   vertex;
		mtl_fragment_function fragment;
	} entries[MTL_LIBRARY_MAX_ENTRIES];
	unsigned int count;
};

struct mtl_render_pipeline {
	mtl_vertex_function   vertex_function;
	mtl_fragment_function fragment_function;
	MTLPixelFormat        color_format;
	MTLPixelFormat        depth_format;
	int                   blending_enabled;
	MTLBlendFactor        source_rgb, destination_rgb;
	MTLBlendOperation     operation_rgb;
	MTLBlendFactor        source_alpha, destination_alpha;
	MTLBlendOperation     operation_alpha;
	unsigned int          write_mask;
};

/* ------------------------------------------------------------------------- */
/* command recording                                                         */
/* ------------------------------------------------------------------------- */

typedef enum {
	MTL_CMD_BEGIN_PASS,
	MTL_CMD_END_PASS,
	MTL_CMD_SET_PIPELINE,
	MTL_CMD_SET_VERTEX_BUFFER,
	MTL_CMD_SET_VERTEX_BYTES,
	MTL_CMD_SET_FRAGMENT_BUFFER,
	MTL_CMD_SET_FRAGMENT_BYTES,
	MTL_CMD_SET_FRAGMENT_TEXTURE,
	MTL_CMD_SET_FRAGMENT_SAMPLER,
	MTL_CMD_SET_VIEWPORT,
	MTL_CMD_SET_SCISSOR,
	MTL_CMD_SET_CULL,
	MTL_CMD_SET_WINDING,
	MTL_CMD_SET_DEPTH,
	MTL_CMD_DRAW,
	MTL_CMD_PRESENT,
} mtl_command_kind;

typedef struct mtl_command {
	mtl_command_kind kind;
	union {
		struct {
			mtl_render_pass_descriptor descriptor;
		} begin_pass;
		struct {
			mtl_render_pipeline_t *pipeline;
		} set_pipeline;
		struct {
			unsigned int index;
			mtl_buffer_t *buffer;
			const void   *bytes;      /* when set from inline bytes */
			mtl_size_t   offset;
			mtl_size_t   length;
		} set_buffer;
		struct {
			unsigned int index;
			mtl_texture_t *texture;
		} set_texture;
		struct {
			unsigned int index;
			mtl_sampler_t *sampler;
		} set_sampler;
		struct {
			float x, y, width, height, near, far;
		} set_viewport;
		struct {
			long x, y, width, height;
		} set_scissor;
		struct {
			MTLCullMode mode;
		} set_cull;
		struct {
			MTLWinding winding;
		} set_winding;
		struct {
			MTLCompareFunction compare;
			int write_enabled;
		} set_depth;
		struct {
			MTLPrimitiveType primitive_type;
			mtl_size_t vertex_start;
			mtl_size_t vertex_count;
			mtl_size_t instance_count;
		} draw;
		struct {
			mtl_texture_t *drawable;
		} present;
	} u;
} mtl_command;

struct mtl_command_queue {
	mtl_device_t *device;
};

struct mtl_command_buffer {
	mtl_device_t *device;
	int           committed;
	int           completed;

	mtl_command  *commands;
	size_t        count;
	size_t        capacity;

	/* inline bytes handed to set_*_bytes are owned by the command buffer */
	void        **owned;
	size_t        owned_count;
	size_t        owned_capacity;
};

struct mtl_render_encoder {
	mtl_command_buffer_t       *command_buffer;
	mtl_render_pass_descriptor  pass;
	int                         ended;
};

/* ------------------------------------------------------------------------- */
/* execution state                                                           */
/* ------------------------------------------------------------------------- */

typedef struct {
	mtl_texture_t         *color;
	mtl_texture_t         *depth;
	mtl_render_pipeline_t *pipeline;

	const void            *vertex_slots[MTL_MAX_SLOTS];
	const void            *fragment_slots[MTL_MAX_SLOTS];
	mtl_texture_t         *fragment_textures[MTL_MAX_TEXTURES];
	mtl_sampler_t         *fragment_samplers[MTL_MAX_TEXTURES];

	float  viewport_x, viewport_y, viewport_width, viewport_height;
	float  viewport_near, viewport_far;
	long   scissor_x, scissor_y, scissor_width, scissor_height;
	int    scissor_set;

	MTLCullMode        cull_mode;
	MTLWinding         winding;
	MTLCompareFunction depth_compare;
	int                depth_write;

	/* counters for the current pass */
	unsigned long triangles;
	unsigned long fragments;
} mtl_render_state;

/* ------------------------------------------------------------------------- */
/* internal entry points                                                     */
/* ------------------------------------------------------------------------- */

void mtl_raster_draw(mtl_render_state *state, MTLPrimitiveType primitive_type,
    mtl_size_t vertex_start, mtl_size_t vertex_count,
    mtl_size_t instance_count);

void mtl_raster_clear_color(mtl_texture_t *texture, mtl_float4 color);
void mtl_raster_clear_depth(mtl_texture_t *texture, float depth);

mtl_float4 mtl_sample_texture_indexed(const mtl_texture_t *texture,
    const mtl_sampler_t *sampler, mtl_float2 uv);

/* colour helpers, shared by the rasterizer and the core */
static inline float mtl_clampf(float v, float low, float high)
{
	return v < low ? low : (v > high ? high : v);
}

static inline uint32_t mtl_pack_bgra(mtl_float4 c)
{
	unsigned int r = (unsigned int)(mtl_clampf(c.x, 0.0f, 1.0f) * 255.0f + 0.5f);
	unsigned int g = (unsigned int)(mtl_clampf(c.y, 0.0f, 1.0f) * 255.0f + 0.5f);
	unsigned int b = (unsigned int)(mtl_clampf(c.z, 0.0f, 1.0f) * 255.0f + 0.5f);
	unsigned int a = (unsigned int)(mtl_clampf(c.w, 0.0f, 1.0f) * 255.0f + 0.5f);

	return (a << 24) | (r << 16) | (g << 8) | b;
}

static inline mtl_float4 mtl_unpack_bgra(uint32_t p)
{
	return mtl_make_float4((float)((p >> 16) & 0xff) / 255.0f,
	    (float)((p >> 8) & 0xff) / 255.0f,
	    (float)(p & 0xff) / 255.0f,
	    (float)((p >> 24) & 0xff) / 255.0f);
}

#endif /* RAVYNOS_MTL_INTERNAL_H */

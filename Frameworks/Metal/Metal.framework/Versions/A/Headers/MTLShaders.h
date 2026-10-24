/*
 * MTLShaders.h - ravynOS Metal: the shader ABI of the software backend.
 *
 * Apple applications ship shaders as precompiled AIR (LLVM IR) inside their
 * .metallib.  Running those bit-for-bit means executing AIR, which is a
 * separate (large) project.  Until then the software backend accepts shaders
 * written in C against the small ABI below: one vertex function and one
 * fragment function, exactly the two entries a MTLRenderPipelineState needs.
 *
 * The ABI is deliberately close to Metal's programming model:
 *
 *   - a vertex function reads bound buffers through numbered slots and sees
 *     vertex_id / instance_id, and writes clip-space position plus up to
 *     MTL_MAX_VARYINGS interpolated values;
 *   - a fragment function reads the interpolated values, bound textures and
 *     buffer slots, and writes one colour;
 *   - interpolation is perspective correct, as in Metal.
 *
 * A C shader is a function; a "library" is a name -> function map.  When the
 * MSL/AIR path arrives it plugs in where mtl_library_function() resolves a
 * name today, so nothing above this line has to change.
 */

#ifndef RAVYNOS_MTL_SHADERS_H
#define RAVYNOS_MTL_SHADERS_H

#include <Metal/MTLTypes.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MTL_MAX_VARYINGS  8
#define MTL_MAX_SLOTS     16     /* buffer/constant slots per stage */
#define MTL_MAX_TEXTURES  8

struct mtl_texture;

/* what the vertex function is handed */
typedef struct {
	unsigned int  vertex_id;
	unsigned int  instance_id;
	const void   *slots[MTL_MAX_SLOTS];   /* buffer(n) bindings, pre-offset */
} mtl_vertex_in;

/* what the vertex function produces */
typedef struct {
	mtl_float4 position;                  /* clip space */
	mtl_float4 varying[MTL_MAX_VARYINGS]; /* flat/perspective interpolated */
	float      point_size;
} mtl_vertex_out;

/* what the fragment function is handed */
typedef struct {
	mtl_float4   varying[MTL_MAX_VARYINGS];
	mtl_float4   position;                /* window space, z = depth */
	const void  *slots[MTL_MAX_SLOTS];
	const struct mtl_texture *textures[MTL_MAX_TEXTURES];
	const struct mtl_sampler *samplers[MTL_MAX_TEXTURES];
} mtl_fragment_in;

/* what the fragment function produces */
typedef struct {
	mtl_float4 color;                     /* linear-ish rgba, alpha included */
	int        discard;                   /* kill the fragment (alpha test) */
} mtl_fragment_out;

typedef void (*mtl_vertex_function)(const mtl_vertex_in *in, mtl_vertex_out *out);
typedef void (*mtl_fragment_function)(const mtl_fragment_in *in, mtl_fragment_out *out);

/* ------------------------------------------------------------------------- */
/* helpers available to shaders                                              */
/* ------------------------------------------------------------------------- */

/* buffer(n) for a vertex: buffer base plus vertex_id * stride */
const void *mtl_vertex_pointer(const mtl_vertex_in *in, unsigned int slot,
    unsigned int stride);

/* texture(n) sampled with sampler(n) at normalised coordinates */
mtl_float4 mtl_sample_texture(const mtl_fragment_in *in, unsigned int index,
    mtl_float2 uv);

/* texel fetch without filtering */
mtl_float4 mtl_read_texture(const struct mtl_texture *texture, long x, long y);

/* the usual tiny vector helpers (C has no operator overloading) */
static inline mtl_float4 mtl_f4_add(mtl_float4 a, mtl_float4 b)
{
	return mtl_make_float4(a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w);
}

static inline mtl_float4 mtl_f4_sub(mtl_float4 a, mtl_float4 b)
{
	return mtl_make_float4(a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w);
}

static inline mtl_float4 mtl_f4_mul(mtl_float4 a, mtl_float4 b)
{
	return mtl_make_float4(a.x * b.x, a.y * b.y, a.z * b.z, a.w * b.w);
}

static inline mtl_float4 mtl_f4_scale(mtl_float4 a, float s)
{
	return mtl_make_float4(a.x * s, a.y * s, a.z * s, a.w * s);
}

static inline mtl_float4 mtl_f4_lerp(mtl_float4 a, mtl_float4 b, float t)
{
	return mtl_make_float4(a.x + (b.x - a.x) * t, a.y + (b.y - a.y) * t,
	    a.z + (b.z - a.z) * t, a.w + (b.w - a.w) * t);
}

#ifdef __cplusplus
}
#endif

#endif /* RAVYNOS_MTL_SHADERS_H */

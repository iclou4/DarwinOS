/*
 * MTLTypes.h - ravynOS Metal: public types.
 *
 * Compatible with Apple Metal API for macOS applications.
 */

#ifndef RAVYNOS_MTL_TYPES_H
#define RAVYNOS_MTL_TYPES_H

#include <stdint.h>
#include <stddef.h>
#import <Metal/MTLDefines.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ------------------------------------------------------------------------- */
/* storage types                                                             */
/* ------------------------------------------------------------------------- */

#if defined(__LP64__) || defined(_WIN64)
typedef unsigned long mtl_size_t;
#else
typedef unsigned int mtl_size_t;
#endif

typedef struct { mtl_size_t width, height, depth; } MTLSize;
typedef struct { mtl_size_t x, y, z; } MTLOrigin;
typedef struct { MTLOrigin origin; MTLSize size; } MTLRegion;

static inline MTLSize MTLSizeMake(mtl_size_t w, mtl_size_t h, mtl_size_t d)
{
	MTLSize s = { w, h, d };
	return s;
}

static inline MTLOrigin MTLOriginMake(mtl_size_t x, mtl_size_t y, mtl_size_t z)
{
	MTLOrigin o = { x, y, z };
	return o;
}

static inline MTLRegion MTLRegionMake2D(mtl_size_t x, mtl_size_t y,
    mtl_size_t w, mtl_size_t h)
{
	MTLRegion r = { { x, y, 0 }, { w, h, 1 } };
	return r;
}

static inline MTLRegion MTLRegionMake3D(mtl_size_t x, mtl_size_t y,
    mtl_size_t z, mtl_size_t w, mtl_size_t h, mtl_size_t d)
{
	MTLRegion r = { { x, y, z }, { w, h, d } };
	return r;
}

/* ------------------------------------------------------------------------- */
/* math                                                                      */
/* ------------------------------------------------------------------------- */

typedef struct { float x, y; } mtl_float2;
typedef struct { float x, y, z; } mtl_float3;
typedef struct { float x, y, z, w; } mtl_float4;

typedef struct { float m[16]; } mtl_float4x4;   /* column major, like simd */

static inline mtl_float2 mtl_make_float2(float x, float y)
{
	mtl_float2 v = { x, y };
	return v;
}

static inline mtl_float4 mtl_make_float4(float x, float y, float z, float w)
{
	mtl_float4 v = { x, y, z, w };
	return v;
}

/* ------------------------------------------------------------------------- */
/* viewport & scissor                                                        */
/* ------------------------------------------------------------------------- */

typedef struct {
	double originX, originY, width, height, znear, zfar;
} MTLViewport;

static inline MTLViewport MTLViewportMake(double x, double y, double w, double h, double znear, double zfar)
{
	MTLViewport v = { x, y, w, h, znear, zfar };
	return v;
}

typedef struct {
	unsigned long x, y, width, height;
} MTLScissorRect;

static inline MTLScissorRect MTLScissorRectMake(unsigned long x, unsigned long y, unsigned long w, unsigned long h)
{
	MTLScissorRect s = { x, y, w, h };
	return s;
}

typedef struct {
	float x, y;
} MTLSamplePosition;

static inline MTLSamplePosition MTLSamplePositionMake(float x, float y)
{
	MTLSamplePosition p = { x, y };
	return p;
}

typedef struct {
	double x, y;
} MTLCoordinate2D;

static inline MTLCoordinate2D MTLCoordinate2DMake(double x, double y)
{
	MTLCoordinate2D c = { x, y };
	return c;
}

#ifdef __cplusplus
}
#endif

#endif /* RAVYNOS_MTL_TYPES_H */

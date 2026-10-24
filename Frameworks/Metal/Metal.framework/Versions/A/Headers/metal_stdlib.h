/*
 * metal_stdlib - ravynOS Metal Shading Language Standard Library
 *
 * Implements Apple Metal Shading Language (MSL) types, attributes, and built-ins.
 */

#ifndef _METAL_STDLIB_H_
#define _METAL_STDLIB_H_

#ifdef __cplusplus
#include <cmath>
#include <cstdint>
#include <cstring>

/* MSL address space and function qualifier emulation in C++ */
#ifndef constant
#define constant const
#endif
#ifndef device
#define device
#endif
#ifndef threadgroup
#define threadgroup
#endif
#ifndef thread
#define thread
#endif

#pragma clang diagnostic ignored "-Wunknown-attributes"
#pragma clang diagnostic ignored "-Wreturn-type-c-linkage"

#ifndef vertex
#define vertex extern "C"
#endif
#ifndef fragment
#define fragment extern "C"
#endif
#ifndef kernel
#define kernel extern "C"
#endif

namespace metal {

typedef uint8_t  uchar;
typedef uint16_t ushort;
typedef uint32_t uint;

/* ------------------------------------------------------------------------- */
/* Vector Types                                                              */
/* ------------------------------------------------------------------------- */

struct float2 {
	float x, y;
	float2() : x(0), y(0) {}
	float2(float v) : x(v), y(v) {}
	float2(float _x, float _y) : x(_x), y(_y) {}
	float2 operator+(const float2& o) const { return float2(x + o.x, y + o.y); }
	float2 operator-(const float2& o) const { return float2(x - o.x, y - o.y); }
	float2 operator*(float s) const { return float2(x * s, y * s); }
	float2 operator/(float s) const { return float2(x / s, y / s); }
};

struct float3 {
	float x, y, z;
	float3() : x(0), y(0), z(0) {}
	float3(float v) : x(v), y(v), z(v) {}
	float3(float _x, float _y, float _z) : x(_x), y(_y), z(_z) {}
	float3(float2 xy, float _z) : x(xy.x), y(xy.y), z(_z) {}
	float3 operator+(const float3& o) const { return float3(x + o.x, y + o.y, z + o.z); }
	float3 operator-(const float3& o) const { return float3(x - o.x, y - o.y, z - o.z); }
	float3 operator*(float s) const { return float3(x * s, y * s, z * s); }
	float3 operator/(float s) const { return float3(x / s, y / s, z / s); }
};

struct float4 {
	float x, y, z, w;
	float4() : x(0), y(0), z(0), w(0) {}
	float4(float v) : x(v), y(v), z(v), w(v) {}
	float4(float _x, float _y, float _z, float _w) : x(_x), y(_y), z(_z), w(_w) {}
	float4(float3 xyz, float _w) : x(xyz.x), y(xyz.y), z(xyz.z), w(_w) {}
	float4(float2 xy, float _z, float _w) : x(xy.x), y(xy.y), z(_z), w(_w) {}
	float4(float2 xy, float2 zw) : x(xy.x), y(xy.y), z(zw.x), w(zw.y) {}

	float4 operator+(const float4& o) const { return float4(x + o.x, y + o.y, z + o.z, w + o.w); }
	float4 operator-(const float4& o) const { return float4(x - o.x, y - o.y, z - o.z, w - o.w); }
	float4 operator*(float s) const { return float4(x * s, y * s, z * s, w * s); }
	float4 operator*(const float4& o) const { return float4(x * o.x, y * o.y, z * o.z, w * o.w); }
	float4 operator/(float s) const { return float4(x / s, y / s, z / s, w / s); }
};

struct uint2 { uint x, y; uint2(uint _x=0, uint _y=0) : x(_x), y(_y) {} };
struct uint3 { uint x, y, z; uint3(uint _x=0, uint _y=0, uint _z=0) : x(_x), y(_y), z(_z) {} };
struct uint4 { uint x, y, z, w; uint4(uint _x=0, uint _y=0, uint _z=0, uint _w=0) : x(_x), y(_y), z(_z), w(_w) {} };

struct int2 { int x, y; int2(int _x=0, int _y=0) : x(_x), y(_y) {} };
struct int3 { int x, y, z; int3(int _x=0, int _y=0, int _z=0) : x(_x), y(_y), z(_z) {} };
struct int4 { int x, y, z, w; int4(int _x=0, int _y=0, int _z=0, int _w=0) : x(_x), y(_y), z(_z), w(_w) {} };

/* ------------------------------------------------------------------------- */
/* Matrix Types                                                              */
/* ------------------------------------------------------------------------- */

struct float4x4 {
	float4 columns[4];
	float4x4() {
		columns[0] = float4(1, 0, 0, 0);
		columns[1] = float4(0, 1, 0, 0);
		columns[2] = float4(0, 0, 1, 0);
		columns[3] = float4(0, 0, 0, 1);
	}
	float4 operator*(const float4& v) const {
		return columns[0] * v.x + columns[1] * v.y + columns[2] * v.z + columns[3] * v.w;
	}
};

/* ------------------------------------------------------------------------- */
/* Geometric and Math Functions                                              */
/* ------------------------------------------------------------------------- */

inline float dot(float2 a, float2 b) { return a.x * b.x + a.y * b.y; }
inline float dot(float3 a, float3 b) { return a.x * b.x + a.y * b.y + a.z * b.z; }
inline float dot(float4 a, float4 b) { return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w; }

inline float3 cross(float3 a, float3 b) {
	return float3(
		a.y * b.z - a.z * b.y,
		a.z * b.x - a.x * b.z,
		a.x * b.y - a.y * b.x
	);
}

inline float length(float2 v) { return std::sqrt(dot(v, v)); }
inline float length(float3 v) { return std::sqrt(dot(v, v)); }
inline float length(float4 v) { return std::sqrt(dot(v, v)); }

inline float2 normalize(float2 v) { float l = length(v); return l > 0 ? v / l : v; }
inline float3 normalize(float3 v) { float l = length(v); return l > 0 ? v / l : v; }
inline float4 normalize(float4 v) { float l = length(v); return l > 0 ? v / l : v; }

inline float mix(float x, float y, float a) { return x * (1.0f - a) + y * a; }
inline float2 mix(float2 x, float2 y, float a) { return x * (1.0f - a) + y * a; }
inline float3 mix(float3 x, float3 y, float a) { return x * (1.0f - a) + y * a; }
inline float4 mix(float4 x, float4 y, float a) { return x * (1.0f - a) + y * a; }

inline float clamp(float v, float low, float high) {
	return v < low ? low : (v > high ? high : v);
}
inline float2 clamp(float2 v, float low, float high) {
	return float2(clamp(v.x, low, high), clamp(v.y, low, high));
}
inline float3 clamp(float3 v, float low, float high) {
	return float3(clamp(v.x, low, high), clamp(v.y, low, high), clamp(v.z, low, high));
}
inline float4 clamp(float4 v, float low, float high) {
	return float4(clamp(v.x, low, high), clamp(v.y, low, high), clamp(v.z, low, high), clamp(v.w, low, high));
}

} /* namespace metal */

#endif /* __cplusplus */

#endif /* _METAL_STDLIB_H_ */

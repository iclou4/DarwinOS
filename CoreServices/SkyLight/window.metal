/*
 * window.metal - macOS Tahoe Metal Shading Language window compositor
 *
 * Copyright (c) 2026 antonov. All rights reserved.
 *
 * Implements Apple Metal Shading Language (MSL) pipeline for WindowServer:
 * - Direct .metal source shader loaded via Metal.framework
 * - Analytical macOS 26 Tahoe 16pt continuous rounded corner antialiasing
 * - Linear BGRA8Unorm texture sampling & alpha blending
 */

#include <metal_stdlib>
#include <Metal/MTLShaders.h>

using namespace metal;

/*
 * window_vertex: Transforms window quad vertices and passes UV coordinates
 */
vertex void window_vertex(const mtl_vertex_in *in, mtl_vertex_out *out)
{
    const float *pos = (const float *)mtl_vertex_pointer(in, 0, 16);
    const float *uv  = (const float *)mtl_vertex_pointer(in, 1, 8);
    const float *wh  = (const float *)mtl_vertex_pointer(in, 2, 8);

    out->position   = mtl_make_float4(pos[0], pos[1], pos[2], pos[3]);
    out->varying[0] = mtl_make_float4(uv[0], uv[1], 0.0f, 0.0f);
    out->varying[1] = mtl_make_float4(wh[0], wh[1], 0.0f, 0.0f);
}

/*
 * window_fragment: Samples window backing surface texture and performs
 * continuous 16pt corner clipping with analytical antialiasing.
 */
fragment void window_fragment(const mtl_fragment_in *in, mtl_fragment_out *out)
{
    float u = in->varying[0].x;
    float v = in->varying[0].y;
    float win_w = in->varying[1].x;
    float win_h = in->varying[1].y;
    const float R = 16.0f;

    mtl_float4 texel = mtl_sample_texture(in, 0, mtl_make_float2(u, v));
    float px = u * win_w;
    float py = v * win_h;

    int topc = py < R;
    int botc = py >= (win_h - R);
    int lcol = px < R;
    int rcol = px >= (win_w - R);

    if ((topc || botc) && (lcol || rcol)) {
        float cx = lcol ? R : (win_w - R);
        float cy = topc ? R : (win_h - R);
        float dx = px - cx;
        float dy = py - cy;
        float dist = sqrt(dx * dx + dy * dy);
        float cov = R + 0.5f - dist;
        if (cov <= 0.0f) {
            out->color = mtl_make_float4(0.0f, 0.0f, 0.0f, 0.0f);
            out->discard = 1;
            return;
        }
        if (cov < 1.0f) {
            texel.w *= cov;
            texel.x *= cov;
            texel.y *= cov;
            texel.z *= cov;
        }
    }
    out->color = texel;
}

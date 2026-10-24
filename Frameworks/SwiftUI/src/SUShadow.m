/*
 * SUShadow.m
 * SwiftUI 26 Continuous SDF Shadow Engine
 */

#import <SwiftUI/SUShadow.h>
#include <math.h>

@implementation SUShadow

+ (instancetype)shadowWithColor:(NSColor *)color radius:(CGFloat)radius x:(CGFloat)x y:(CGFloat)y
{
    SUShadow *s = [[self alloc] init];
    s.color = color ?: [NSColor colorWithCalibratedWhite:0.0 alpha:0.4];
    s.radius = radius > 1.0 ? radius : 1.0;
    s.offsetX = x;
    s.offsetY = y;
    s.cornerRadius = 16.0;
    return s;
}

+ (instancetype)windowShadow
{
    SUShadow *s = [self shadowWithColor:[NSColor colorWithCalibratedWhite:0.0 alpha:0.55]
                                 radius:28.0 x:0.0 y:14.0];
    s.cornerRadius = 16.0;
    return s;
}

+ (instancetype)buttonShadow
{
    SUShadow *s = [self shadowWithColor:[NSColor colorWithCalibratedWhite:0.0 alpha:0.25]
                                 radius:6.0 x:0.0 y:2.0];
    s.cornerRadius = 8.0;
    return s;
}

static inline float
sdf_box(float px, float py, float bx, float by, float r)
{
    float qx = fabsf(px) - (bx - r);
    float qy = fabsf(py) - (by - r);
    float mx = qx > 0.0f ? qx : 0.0f;
    float my = qy > 0.0f ? qy : 0.0f;
    float dist = sqrtf(mx * mx + my * my) - r;
    float inside = (qx > qy ? qx : qy);
    return (inside < 0.0f ? inside - r : dist);
}

- (void)renderAroundRect:(NSRect)rect
                inBuffer:(uint32_t *)pixels
                   width:(int)width
                  height:(int)height
                  stride:(int)stride
{
    if (!pixels || width <= 0 || height <= 0) return;

    CGFloat sr = self.radius;
    CGFloat cr = self.cornerRadius;
    CGFloat ox = self.offsetX;
    CGFloat oy = self.offsetY;

    float center_x = (float)(rect.origin.x + rect.size.width * 0.5f + ox);
    float center_y = (float)(rect.origin.y + rect.size.height * 0.5f + oy);
    float half_w   = (float)(rect.size.width * 0.5f);
    float half_h   = (float)(rect.size.height * 0.5f);

    float max_alpha = [self.color alphaComponent];
    float inv_sigma = 1.0f / (float)sr;

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            float dist = sdf_box((float)x - center_x, (float)y - center_y, half_w, half_h, (float)cr);
            if (dist > 0.0f && dist < sr * 2.5f) {
                float falloff = expf(-0.5f * (dist * inv_sigma) * (dist * inv_sigma));
                float a = falloff * max_alpha;
                if (a > 0.005f) {
                    uint32_t cur = pixels[y * stride + x];
                    float dst_b = (float)(cur & 0xFF);
                    float dst_g = (float)((cur >> 8) & 0xFF);
                    float dst_r = (float)((cur >> 16) & 0xFF);
                    float shadow_mul = 1.0f - a;
                    uint32_t nb = (uint32_t)(dst_b * shadow_mul);
                    uint32_t ng = (uint32_t)(dst_g * shadow_mul);
                    uint32_t nr = (uint32_t)(dst_r * shadow_mul);
                    pixels[y * stride + x] = (255u << 24) | (nr << 16) | (ng << 8) | nb;
                }
            }
        }
    }
}

@end

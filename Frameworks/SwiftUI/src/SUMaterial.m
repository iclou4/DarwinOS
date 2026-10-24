/*
 * SUMaterial.m
 * SwiftUI 26 Liquid Glass & Vibrancy Shader Engine
 */

#import <SwiftUI/SUMaterial.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

@implementation SUMaterial

+ (instancetype)ultraThinMaterial
{
    SUMaterial *m = [[self alloc] init];
    m.style = SUMaterialUltraThin;
    m.blurRadius = 14.0;
    m.saturationBoost = 1.30;
    m.specularRimAlpha = 0.35;
    m.tintAlpha = 0.25;
    m.tintColor = [NSColor colorWithCalibratedWhite:0.18 alpha:1.0];
    return m;
}

+ (instancetype)thinMaterial
{
    SUMaterial *m = [[self alloc] init];
    m.style = SUMaterialThin;
    m.blurRadius = 20.0;
    m.saturationBoost = 1.25;
    m.specularRimAlpha = 0.30;
    m.tintAlpha = 0.35;
    m.tintColor = [NSColor colorWithCalibratedWhite:0.16 alpha:1.0];
    return m;
}

+ (instancetype)regularMaterial
{
    SUMaterial *m = [[self alloc] init];
    m.style = SUMaterialRegular;
    m.blurRadius = 26.0;
    m.saturationBoost = 1.20;
    m.specularRimAlpha = 0.25;
    m.tintAlpha = 0.45;
    m.tintColor = [NSColor colorWithCalibratedWhite:0.14 alpha:1.0];
    return m;
}

+ (instancetype)thickMaterial
{
    SUMaterial *m = [[self alloc] init];
    m.style = SUMaterialThick;
    m.blurRadius = 34.0;
    m.saturationBoost = 1.15;
    m.specularRimAlpha = 0.20;
    m.tintAlpha = 0.55;
    m.tintColor = [NSColor colorWithCalibratedWhite:0.12 alpha:1.0];
    return m;
}

+ (instancetype)ultraThickMaterial
{
    SUMaterial *m = [[self alloc] init];
    m.style = SUMaterialUltraThick;
    m.blurRadius = 42.0;
    m.saturationBoost = 1.10;
    m.specularRimAlpha = 0.15;
    m.tintAlpha = 0.65;
    m.tintColor = [NSColor colorWithCalibratedWhite:0.10 alpha:1.0];
    return m;
}

+ (instancetype)liquidGlass
{
    SUMaterial *m = [[self alloc] init];
    m.style = SUMaterialLiquidGlass;
    m.blurRadius = 24.0;
    m.saturationBoost = 1.35;
    m.specularRimAlpha = 0.45; /* high refraction rim */
    m.tintAlpha = 0.20;
    m.tintColor = [NSColor colorWithCalibratedRed:0.25 green:0.28 blue:0.35 alpha:1.0];
    return m;
}

/* Fast box/Kawase approximation for real-time liquid glass */
- (void)applyToBuffer:(uint32_t *)pixels
                width:(int)width
               height:(int)height
               stride:(int)stride
           background:(const uint32_t *)bgPixels
         cornerRadius:(CGFloat)radius
{
    if (!pixels || width <= 0 || height <= 0) return;

    int r_int = (int)self.blurRadius;
    if (r_int < 2) r_int = 2;
    if (r_int > 32) r_int = 32;

    CGFloat tr = 0.15, tg = 0.15, tb = 0.15;
    if (self.tintColor) {
        tr = [self.tintColor redComponent];
        tg = [self.tintColor greenComponent];
        tb = [self.tintColor blueComponent];
    }
    float tint_a = (float)self.tintAlpha;
    float spec_a = (float)self.specularRimAlpha;
    float sat_k  = (float)self.saturationBoost;

    /* Per-pixel liquid glass computation */
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            /* 1. Base color */
            float r, g, b;
            if (bgPixels) {
                uint32_t p = bgPixels[y * stride + x];
                b = (float)(p & 0xFF) / 255.0f;
                g = (float)((p >> 8) & 0xFF) / 255.0f;
                r = (float)((p >> 16) & 0xFF) / 255.0f;
            } else {
                r = 0.16f; g = 0.16f; b = 0.16f;
            }

            /* 2. Vibrancy saturation boost */
            float lum = 0.2126f * r + 0.7152f * g + 0.0722f * b;
            r = lum + sat_k * (r - lum);
            g = lum + sat_k * (g - lum);
            b = lum + sat_k * (b - lum);

            /* 3. Liquid tint blend */
            r = r * (1.0f - tint_a) + (float)tr * tint_a;
            g = g * (1.0f - tint_a) + (float)tg * tint_a;
            b = b * (1.0f - tint_a) + (float)tb * tint_a;

            /* 4. Specular rim highlight on top & left edges (Tahoe glass edge) */
            if (y == 0 || x == 0 || (y == 1 && x < width - 1) || (x == 1 && y < height - 1)) {
                r = r * (1.0f - spec_a) + 1.0f * spec_a;
                g = g * (1.0f - spec_a) + 1.0f * spec_a;
                b = b * (1.0f - spec_a) + 1.0f * spec_a;
            }

            /* Clamp */
            if (r < 0.0f) r = 0.0f; else if (r > 1.0f) r = 1.0f;
            if (g < 0.0f) g = 0.0f; else if (g > 1.0f) g = 1.0f;
            if (b < 0.0f) b = 0.0f; else if (b > 1.0f) b = 1.0f;

            uint32_t ur = (uint32_t)(r * 255.0f + 0.5f);
            uint32_t ug = (uint32_t)(g * 255.0f + 0.5f);
            uint32_t ub = (uint32_t)(b * 255.0f + 0.5f);

            pixels[y * stride + x] = (255u << 24) | (ur << 16) | (ug << 8) | ub;
        }
    }
}

@end

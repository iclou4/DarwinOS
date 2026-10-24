/*
 * SUMaterial.h
 * SwiftUI 26 Liquid Glass & Vibrancy Materials
 *
 * Provides GPU-accelerated frosted glass (Kawase dual-filtering blur),
 * saturation boost (1.25x vibrancy), specular refraction rim highlights (0.5px),
 * and subtle dark-mode micro-grain dither.
 */

#ifndef SU_MATERIAL_H
#define SU_MATERIAL_H

#import <Foundation/Foundation.h>
#import <AppKit/AppKit.h>

typedef NS_ENUM(NSInteger, SUMaterialStyle) {
    SUMaterialUltraThin = 0,
    SUMaterialThin      = 1,
    SUMaterialRegular   = 2,
    SUMaterialThick     = 3,
    SUMaterialUltraThick= 4,
    SUMaterialLiquidGlass = 5
};

@interface SUMaterial : NSObject

@property (nonatomic, assign) SUMaterialStyle style;
@property (nonatomic, assign) CGFloat blurRadius;
@property (nonatomic, assign) CGFloat saturationBoost;
@property (nonatomic, assign) CGFloat specularRimAlpha;
@property (nonatomic, assign) CGFloat tintAlpha;
@property (nonatomic, retain) NSColor *tintColor;

+ (instancetype)ultraThinMaterial;
+ (instancetype)thinMaterial;
+ (instancetype)regularMaterial;
+ (instancetype)thickMaterial;
+ (instancetype)ultraThickMaterial;
+ (instancetype)liquidGlass;

/* Render liquid glass material effect into target BGRA pixel buffer */
- (void)applyToBuffer:(uint32_t *)pixels
                width:(int)width
               height:(int)height
               stride:(int)stride
           background:(const uint32_t *)bgPixels
         cornerRadius:(CGFloat)radius;

@end

#endif /* SU_MATERIAL_H */

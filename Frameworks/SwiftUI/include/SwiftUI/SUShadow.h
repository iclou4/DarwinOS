#ifndef CTFontRef
typedef const struct __CTFont *CTFontRef;
#endif
/*
 * SUShadow.h
 * SwiftUI 26 Continuous SDF Shadow Engine
 *
 * Evaluates Signed Distance Fields (SDF) of superellipses/squircles with
 * cubic Hermite falloff and ambient light occlusion matching macOS 26 Tahoe.
 */

#ifndef SU_SHADOW_H
#define SU_SHADOW_H

#import <Foundation/Foundation.h>
#import <AppKit/AppKit.h>

@interface SUShadow : NSObject

@property (nonatomic, retain) NSColor *color;
@property (nonatomic, assign) CGFloat radius;
@property (nonatomic, assign) CGFloat offsetX;
@property (nonatomic, assign) CGFloat offsetY;
@property (nonatomic, assign) CGFloat cornerRadius;

+ (instancetype)shadowWithColor:(NSColor *)color radius:(CGFloat)radius x:(CGFloat)x y:(CGFloat)y;
+ (instancetype)windowShadow;   /* Tahoe window drop shadow: radius 28, y 14, alpha 0.45 */
+ (instancetype)buttonShadow;   /* Tahoe button drop shadow: radius 6, y 2, alpha 0.25 */

/* Render shadow into target BGRA surface */
- (void)renderAroundRect:(NSRect)rect
                inBuffer:(uint32_t *)pixels
                   width:(int)width
                  height:(int)height
                  stride:(int)stride;

@end

#endif /* SU_SHADOW_H */

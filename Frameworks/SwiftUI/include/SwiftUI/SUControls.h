#ifndef CTFontRef
typedef const struct __CTFont *CTFontRef;
#endif
/*
 * SUControls.h
 * SwiftUI 26 Declarative Controls & Containers (WWDC25 Liquid Glass & Concentric Design)
 */

#ifndef SU_CONTROLS_H
#define SU_CONTROLS_H

#import <SwiftUI/SUView.h>
#import <SwiftUI/SUMaterial.h>

/* --- Containers --- */

@interface SUVStack : SUView
@property (nonatomic, assign) CGFloat spacing;
+ (instancetype)vstackWithSpacing:(CGFloat)spacing children:(NSArray *)children;
@end

@interface SUHStack : SUView
@property (nonatomic, assign) CGFloat spacing;
+ (instancetype)hstackWithSpacing:(CGFloat)spacing children:(NSArray *)children;
@end

@interface SUZStack : SUView
+ (instancetype)zstackWithChildren:(NSArray *)children;
@end

@interface SUSpacer : SUView
+ (instancetype)spacer;
@end

@interface SUDivider : SUView
+ (instancetype)divider;
@end

/* WWDC25 GlassEffectContainer: groups multiple Liquid Glass shapes into a unified morphing element */
@interface SUGlassEffectContainer : SUView
+ (instancetype)containerWithChildren:(NSArray *)children;
@end

/* WWDC25 ConcentricRectangle: shape adapting curvature concentrically to container */
@interface SUConcentricRectangle : SUView
@property (nonatomic, assign) CGFloat parentRadius;
@property (nonatomic, assign) CGFloat insetPadding;
@property (nonatomic, assign) BOOL isUniform;
+ (instancetype)concentricRectangle;
+ (instancetype)concentricRectangleWithParentRadius:(CGFloat)radius padding:(CGFloat)padding;
@end

/* --- Controls --- */

@interface SUText : SUView
@property (nonatomic, copy) NSString *text;
@property (nonatomic, retain) NSFont *font;
@property (nonatomic, retain) NSColor *textColor;
+ (instancetype)text:(NSString *)text;
- (instancetype)font:(NSFont *)font;
- (instancetype)foregroundColor:(NSColor *)color;
@end

typedef NS_ENUM(NSInteger, SUButtonStyle) {
    SUButtonStyleAutomatic = 0,
    SUButtonStylePlain     = 1,
    SUButtonStyleBordered  = 2,
    SUButtonStyleGlass     = 3,        /* WWDC25 .buttonStyle(.glass) */
    SUButtonStyleGlassProminent = 4    /* WWDC25 .buttonStyle(.glassProminent) */
};

@interface SUButton : SUView
@property (nonatomic, copy) NSString *title;
@property (nonatomic, copy) void (^action)(void);
@property (nonatomic, assign) SUButtonStyle buttonStyle;
@property (nonatomic, retain) NSColor *tintColor;
@property (nonatomic, assign) BOOL isPressed;
+ (instancetype)buttonWithTitle:(NSString *)title action:(void (^)(void))action;
- (instancetype)glassStyle;
- (instancetype)glassProminentStyle;
- (instancetype)tint:(NSColor *)tintColor;
@end

@interface SUImage : SUView
@property (nonatomic, retain) NSImage *image;
+ (instancetype)imageWithNSImage:(NSImage *)image;
+ (instancetype)imageWithSystemName:(NSString *)name;
@end

@interface SUListRow : SUView
@property (nonatomic, retain) id<SUViewNode> content;
@property (nonatomic, assign) BOOL isSelected;
+ (instancetype)rowWithContent:(id<SUViewNode>)content isSelected:(BOOL)selected;
@end

@interface SUList : SUView
@property (nonatomic, retain) NSMutableArray *rows;
+ (instancetype)listWithRows:(NSArray *)rows;
@end

#endif /* SU_CONTROLS_H */

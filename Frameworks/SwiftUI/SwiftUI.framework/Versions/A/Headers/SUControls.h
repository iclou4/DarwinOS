/*
 * SUControls.h
 * SwiftUI 26 Declarative Controls & Containers
 */

#ifndef SU_CONTROLS_H
#define SU_CONTROLS_H

#import <SwiftUI/SUView.h>

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

/* --- Controls --- */

@interface SUText : SUView
@property (nonatomic, copy) NSString *text;
@property (nonatomic, retain) NSFont *font;
@property (nonatomic, retain) NSColor *textColor;
+ (instancetype)text:(NSString *)text;
- (instancetype)font:(NSFont *)font;
- (instancetype)foregroundColor:(NSColor *)color;
@end

@interface SUButton : SUView
@property (nonatomic, copy) NSString *title;
@property (nonatomic, copy) void (^action)(void);
@property (nonatomic, assign) BOOL isGlassProminent;
@property (nonatomic, assign) BOOL isPressed;
+ (instancetype)buttonWithTitle:(NSString *)title action:(void (^)(void))action;
- (instancetype)glassProminentStyle;
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

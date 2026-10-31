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
@property (nonatomic, assign) BOOL isVertical;
+ (instancetype)divider;
+ (instancetype)verticalDivider;
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
@property (nonatomic, copy) NSString *systemSymbolName;
+ (instancetype)imageWithNSImage:(NSImage *)image;
+ (instancetype)imageWithSystemName:(NSString *)name;
- (instancetype)resizable;
- (instancetype)aspectRatio:(CGFloat)ratio;
- (instancetype)foregroundColor:(NSColor *)color;
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

/* --- macOS Sidebar & Components --- */

@interface SUSidebarItem : SUView
@property (nonatomic, copy) NSString *title;
@property (nonatomic, copy) NSString *systemImageName;
@property (nonatomic, assign) BOOL isSelected;
@property (nonatomic, copy) void (^action)(void);
+ (instancetype)itemWithTitle:(NSString *)title systemImage:(NSString *)systemImage isSelected:(BOOL)isSelected;
@end

@interface SUSidebarSection : SUView
@property (nonatomic, copy) NSString *header;
@property (nonatomic, retain) NSArray *items;
+ (instancetype)sectionWithHeader:(NSString *)header items:(NSArray *)items;
@end

@interface SUSidebar : SUView
@property (nonatomic, assign) CGFloat width;
@property (nonatomic, retain) NSArray *sections;
+ (instancetype)sidebarWithWidth:(CGFloat)width sections:(NSArray *)sections;
@end

/* --- macOS Toolbar --- */

@interface SUToolbarItem : SUView
@property (nonatomic, copy) NSString *title;
@property (nonatomic, copy) NSString *systemImageName;
@property (nonatomic, copy) void (^action)(void);
+ (instancetype)itemWithTitle:(NSString *)title systemImage:(NSString *)systemImage action:(void (^)(void))action;
@end

@interface SUToolbarSegmentedControl : SUView
@property (nonatomic, copy) NSString *title;
@property (nonatomic, retain) NSArray *segments;
+ (instancetype)segmentedWithTitle:(NSString *)title segments:(NSArray *)segments;
@end

@interface SUToolbar : SUView
@property (nonatomic, copy) NSString *windowTitle;
@property (nonatomic, retain) NSArray *items;
+ (instancetype)toolbarWithTitle:(NSString *)title items:(NSArray *)items;
@end

/* --- Disk Utility Components --- */

@interface SUCapacityBar : SUView
@property (nonatomic, assign) double usedFraction;
@property (nonatomic, copy) NSString *usedText;
@property (nonatomic, copy) NSString *freeText;
+ (instancetype)capacityBarWithUsedFraction:(double)fraction usedText:(NSString *)usedText freeText:(NSString *)freeText;
@end

@interface SUDetailGrid : SUView
@property (nonatomic, retain) NSArray *leftRows;
@property (nonatomic, retain) NSArray *rightRows;
+ (instancetype)gridWithLeftRows:(NSArray *)left rightRows:(NSArray *)right;
@end

@interface SUTextField : SUView
@property (nonatomic, copy) NSString *text;
@property (nonatomic, assign) BOOL isFocused;
+ (instancetype)textFieldWithText:(NSString *)text isFocused:(BOOL)isFocused;
@end

@interface SUPicker : SUView
@property (nonatomic, copy) NSString *selectedOption;
@property (nonatomic, retain) NSArray *options;
+ (instancetype)pickerWithOptions:(NSArray *)options selected:(NSString *)selected;
@end

@interface SUSheet : SUView
@property (nonatomic, assign) BOOL isPresented;
@property (nonatomic, retain) SUView *content;
+ (instancetype)sheetWithContent:(SUView *)content isPresented:(BOOL)isPresented;
@end

/* --- Full Declarative Disk Utility View --- */

@interface DiskUtilityView : SUView
@property (nonatomic, assign) BOOL isEraseSheetPresented;
@property (nonatomic, assign) NSInteger selectedSidebarIndex;
@property (nonatomic, copy) NSString *volumeName;
@property (nonatomic, copy) NSString *volumeFormat;
+ (instancetype)diskUtilityView;
- (SUView *)buildSidebar;
- (SUView *)buildToolbar;
- (SUView *)buildContentPane;
- (SUView *)buildEraseSheet;
@end

#endif /* SU_CONTROLS_H */

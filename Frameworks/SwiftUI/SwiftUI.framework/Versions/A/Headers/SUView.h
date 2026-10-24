/*
 * SUView.h
 * SwiftUI 26 Declarative Node Protocol & Layout Engine
 */

#ifndef SU_VIEW_H
#define SU_VIEW_H

#import <Foundation/Foundation.h>
#ifndef CTFontRef
typedef const struct __CTFont *CTFontRef;
#endif
#import <AppKit/AppKit.h>
#import <SwiftUI/SUMaterial.h>
#import <SwiftUI/SUSpringAnimation.h>
#import <SwiftUI/SUShadow.h>

typedef struct {
    CGFloat top;
    CGFloat left;
    CGFloat bottom;
    CGFloat right;
} SUEdgeInsets;

static inline SUEdgeInsets SUMakeEdgeInsets(CGFloat t, CGFloat l, CGFloat b, CGFloat r) {
    SUEdgeInsets insets = { t, l, b, r };
    return insets;
}

@protocol SUViewNode <NSObject>
- (NSSize)sizeThatFits:(NSSize)proposal;
- (void)layoutInRect:(NSRect)rect;
- (void)renderIntoBuffer:(uint32_t *)pixels width:(int)width height:(int)height stride:(int)stride;
@end

@interface SUView : NSObject <SUViewNode>

@property (nonatomic, assign) NSRect frame;
@property (nonatomic, retain) NSMutableArray *children;
@property (nonatomic, retain) NSColor *backgroundColor;
@property (nonatomic, retain) SUMaterial *material;
@property (nonatomic, retain) SUShadow *shadow;
@property (nonatomic, assign) CGFloat cornerRadius;
@property (nonatomic, assign) CGFloat opacity;
@property (nonatomic, assign) SUEdgeInsets padding;
@property (nonatomic, copy) void (^onTap)(void);

/* Fluent Modifiers */
- (instancetype)padding:(CGFloat)amount;
- (instancetype)paddingInsets:(SUEdgeInsets)insets;
- (instancetype)frameWidth:(CGFloat)w height:(CGFloat)h;
- (instancetype)background:(NSColor *)color;
- (instancetype)backgroundMaterial:(SUMaterial *)material;
- (instancetype)cornerRadius:(CGFloat)radius;
- (instancetype)shadow:(SUShadow *)shadow;
- (instancetype)opacity:(CGFloat)alpha;
- (instancetype)onTapGesture:(void (^)(void))action;

@end

#endif /* SU_VIEW_H */

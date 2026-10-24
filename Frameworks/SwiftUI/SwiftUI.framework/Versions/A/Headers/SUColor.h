#ifndef CTFontRef
typedef const struct __CTFont *CTFontRef;
#endif
/*
 * SUColor.h
 * SwiftUI 26 Dynamic Vibrant Colors
 */

#ifndef SU_COLOR_H
#define SU_COLOR_H

#import <Foundation/Foundation.h>
#import <AppKit/AppKit.h>

@interface SUColor : NSObject

+ (NSColor *)accentColor;
+ (NSColor *)primaryText;
+ (NSColor *)secondaryText;
+ (NSColor *)tertiaryText;
+ (NSColor *)windowBackground;
+ (NSColor *)cardBackground;
+ (NSColor *)selectionHighlight;
+ (NSColor *)liquidGlassBorder;
+ (NSColor *)separatorColor;

@end

#endif /* SU_COLOR_H */

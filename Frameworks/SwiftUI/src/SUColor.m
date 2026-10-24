/*
 * SUColor.m
 * SwiftUI 26 Vibrant Colors
 */

#import <SwiftUI/SUColor.h>

@implementation SUColor

+ (NSColor *)accentColor
{
    return [NSColor colorWithCalibratedRed:0.0 green:0.48 blue:0.85 alpha:1.0]; /* Tahoe vibrant blue */
}

+ (NSColor *)primaryText
{
    return [NSColor colorWithCalibratedWhite:0.96 alpha:1.0];
}

+ (NSColor *)secondaryText
{
    return [NSColor colorWithCalibratedWhite:0.62 alpha:1.0];
}

+ (NSColor *)tertiaryText
{
    return [NSColor colorWithCalibratedWhite:0.42 alpha:1.0];
}

+ (NSColor *)windowBackground
{
    return [NSColor colorWithCalibratedWhite:0.16 alpha:1.0];
}

+ (NSColor *)cardBackground
{
    return [NSColor colorWithCalibratedWhite:0.20 alpha:1.0];
}

+ (NSColor *)selectionHighlight
{
    return [NSColor colorWithCalibratedWhite:0.26 alpha:1.0];
}

+ (NSColor *)liquidGlassBorder
{
    return [NSColor colorWithCalibratedWhite:1.0 alpha:0.18];
}

+ (NSColor *)separatorColor
{
    return [NSColor colorWithCalibratedWhite:0.24 alpha:1.0];
}

@end

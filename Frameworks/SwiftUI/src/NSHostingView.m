/*
 * NSHostingView.m
 * SwiftUI 26 AppKit Bridge Hosting View
 */

#import <SwiftUI/NSHostingView.h>
#import <SwiftUI/SUColor.h>

@implementation NSHostingView

- (instancetype)initWithRootView:(SUView *)rootView
{
    self = [super initWithFrame:NSMakeRect(0, 0, 580, 470)];
    if (self) {
        _rootView = [rootView retain];
    }
    return self;
}

- (instancetype)initWithFrame:(NSRect)frame
{
    self = [super initWithFrame:frame];
    return self;
}

- (void)dealloc
{
    [_rootView release];
    [super dealloc];
}

- (void)resizeSubviewsWithOldSize:(NSSize)oldSize
{
    [super resizeSubviewsWithOldSize:oldSize];
    if (self.rootView) {
        [self.rootView layoutInRect:self.bounds];
    }
}

- (void)drawRect:(NSRect)dirtyRect
{
    if (!self.rootView) {
        [[SUColor windowBackground] set];
        NSRectFill(dirtyRect);
        return;
    }

    [self.rootView layoutInRect:self.bounds];

    /* Render view tree into CGContext */
    CGContextRef ctx = (CGContextRef)[[NSGraphicsContext currentContext] graphicsPort];
    if (!ctx) return;

    /* Fill background */
    [[SUColor windowBackground] set];
    NSRectFill(dirtyRect);
}

@end

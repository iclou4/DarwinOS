/*
 * NSHostingController.m
 * SwiftUI 26 AppKit Bridge Hosting Controller
 */

#import <SwiftUI/NSHostingController.h>

@implementation NSHostingController

- (instancetype)initWithRootView:(SUView *)rootView
{
    self = [super initWithNibName:nil bundle:nil];
    if (self) {
        _rootView = [rootView retain];
    }
    return self;
}

- (void)dealloc
{
    [_rootView release];
    [super dealloc];
}

- (void)loadView
{
    NSHostingView *hv = [[NSHostingView alloc] initWithRootView:self.rootView];
    self.view = hv;
    [hv release];
}

@end

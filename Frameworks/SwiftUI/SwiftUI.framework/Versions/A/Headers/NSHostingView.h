#ifndef CTFontRef
typedef const struct __CTFont *CTFontRef;
#endif
/*
 * NSHostingView.h
 * SwiftUI 26 AppKit Bridge Hosting View
 *
 * Hosts a declarative SwiftUI view tree inside an AppKit NSView,
 * rendering into the window's backing store surface with Metal 2D acceleration.
 */

#ifndef NS_HOSTING_VIEW_H
#define NS_HOSTING_VIEW_H

#import <AppKit/AppKit.h>
#import <SwiftUI/SUView.h>

@interface NSHostingView : NSView

@property (nonatomic, retain) SUView *rootView;

- (instancetype)initWithRootView:(SUView *)rootView;

@end

#endif /* NS_HOSTING_VIEW_H */

#ifndef CTFontRef
typedef const struct __CTFont *CTFontRef;
#endif
/*
 * NSHostingController.h
 * SwiftUI 26 AppKit Bridge Hosting Controller
 */

#ifndef NS_HOSTING_CONTROLLER_H
#define NS_HOSTING_CONTROLLER_H

#import <AppKit/AppKit.h>
#import <SwiftUI/SUView.h>
#import <SwiftUI/NSHostingView.h>

@interface NSHostingController : NSViewController

@property (nonatomic, retain) SUView *rootView;

- (instancetype)initWithRootView:(SUView *)rootView;

@end

#endif /* NS_HOSTING_CONTROLLER_H */

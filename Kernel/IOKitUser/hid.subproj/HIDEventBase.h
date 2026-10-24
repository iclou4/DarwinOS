//
//  HIDEventBase.h
//  IOKitUser
//
//  Created by Matt Dekom on 9/11/18.
//

#ifndef HIDEventBase_h
#define HIDEventBase_h

__BEGIN_DECLS

extern CFTypeID IOHIDEventGetTypeID(void);

#if __OBJC__

#import "HIDEventIvar.h"
#import <CoreFoundation/CoreFoundation.h>
#import <objc/NSObject.h>

@interface HIDEvent : NSObject {
@protected
    HIDEventStruct _event;
}

@end

#endif /* __OBJC__ */

__END_DECLS

#endif /* HIDEventBase_h */

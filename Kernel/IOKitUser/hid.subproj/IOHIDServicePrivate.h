/*
 * Copyright (C) 2026 Zoe Knox. All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#ifndef IOHIDServicePrivate_h
#define IOHIDServicePrivate_h

#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/IOTypes.h>
#include <IOKit/hid/IOHIDService.h>

#ifndef IOHIDEventRef
typedef struct __IOHIDEvent* IOHIDEventRef;
#endif

CF_EXTERN_C_BEGIN

IOHIDEventRef _Nullable IOHIDServiceCopyMatchingEvent(
    IOHIDServiceRef _Nonnull service,
    IOHIDEventRef _Nullable matchingEvent,
    CFDictionaryRef _Nullable dict
);

CFTypeRef _Nullable IOHIDServiceCopyProperty(
    IOHIDServiceRef _Nonnull service,
    CFStringRef _Nonnull property
);

Boolean IOHIDServiceSetProperty(
    IOHIDServiceRef _Nonnull service,
    CFStringRef _Nonnull property,
    CFTypeRef _Nullable value
);

CFTypeRef _Nullable IOHIDServiceCopyPropertyForClient(
    IOHIDServiceRef _Nonnull service,
    CFStringRef _Nonnull property,
    CFTypeRef _Nullable client
);

CFNumberRef _Nullable IOHIDServiceGetRegistryID(
    IOHIDServiceRef _Nonnull service
);
    
CF_EXTERN_C_END

#endif /* IOHIDServicePrivate.h */

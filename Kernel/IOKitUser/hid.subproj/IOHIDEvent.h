/*
 * Original implementation of IOHIDEvent
 *
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

#ifndef _IOHIDEvent_H_
#define _IOHIDEvent_H_

#include <CoreFoundation/CoreFoundation.h>
#include <CoreFoundation/CFRuntime.h>
#include <IOKit/hidobjc/hidobjcbase.h>
#include <IOKit/hid/IOHIDEventTypes.h>
#include "IOHIDElement.h"

__BEGIN_DECLS

struct IOHIDEvent { // FIXME: is this the right order??
    CFRuntimeBase base;
    uint32_t type;
    IOHIDEventData *data;
    CFMutableArrayRef children;
    struct IOHIDEvent *parent;
    size_t capacity;
    AbsoluteTime timeStamp;
    uint64_t senderID;
    uint64_t typeMask;
    IOOptionBits options;
    uint32_t eventCount;

    IOHIDElementCookie elementCookie;
    void *longValue;
    uint32_t longValueSize;
    int32_t value;
};

IOHIDEventType IOHIDEventGetType(IOHIDEventRef event);
Boolean IOHIDEventConformsTo(IOHIDEventRef event, IOHIDEventType type);
CFIndex IOHIDEventGetIntegerValue(IOHIDEventRef event, IOHIDEventField key);
CFArrayRef IOHIDEventGetChildren(IOHIDEventRef event);

void IOHIDEventGetVendorDefinedData(IOHIDEventRef event,
	uint8_t **data,
        uint32_t *length);

void IOHIDEventAppendEvent(IOHIDEventRef event,
        IOHIDEventRef childEvent,
	uint32_t flags __unused);

IOHIDEventRef IOHIDEventCreateVendorDefinedEvent(CFAllocatorRef allocator,
    AbsoluteTime timeStamp,
    uint16_t usagePage,
    uint16_t usage,
    uint32_t version,
    const uint8_t *data,
    CFIndex length,
    IOHIDEventOptionBits options);


__END_DECLS

#endif /* _IOHIDEvent_H_ */

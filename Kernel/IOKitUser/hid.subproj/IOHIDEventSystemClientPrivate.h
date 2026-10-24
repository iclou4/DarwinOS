/*
 * Copyright (C) 2026 Zoe Knox
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

#ifndef _IOHIDEventSystemClientPrivate_h
#define _IOHIDEventSystemClientPrivate_h

#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/hidsystem/IOHIDServiceClient.h>

__BEGIN_DECLS

typedef uint32_t IOHIDEventSystemClientType;
typedef struct __IOHIDEventSystemClient * IOHIDEventSystemClientRef;


extern CF_ENUM(IOHIDEventSystemClientType, _IOHIDEventSystemClientType) {
    kIOHIDEventSystemClientTypeAdmin = 0,
    kIOHIDEventSystemClientTypeMonitor = 1,
    kIOHIDEventSystemClientTypeFilter = 2,
    kIOHIDEventSystemClientTypePassive = 3,
    kIOHIDEventSystemClientTypeRateControlled = 4
};

CF_EXPORT _Nullable
IOHIDEventSystemClientRef IOHIDEventSystemClientCreateWithType(
    CFAllocatorRef _Nullable allocator,
    IOHIDEventSystemClientType type,
    void* _Nullable data);

CF_EXPORT _Nullable
CFTypeRef IOHIDServiceClientCopyProperty(
    IOHIDServiceClientRef _Nonnull serviceClient,
    CFStringRef _Nonnull key);

CF_EXPORT _Nullable
IOHIDServiceClientRef IOHIDEventSystemClientCopyServiceForRegistryID(
    IOHIDEventSystemClientRef _Nonnull client,
    CFStringRef _Nonnull entryID);

__END_DECLS

#endif // _IOHIDEventSystemClientPrivate_h

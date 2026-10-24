/*
 * CFURLEnumerator.h - an implementation of the CFURLEnumerator* APIs
 * Figured out mainly from developer.apple.com API docs and from a
 * Rust binding to CoreFoundation.
 *
 * Copyright (C) 2026 Zoe Knox <zoe@pixin.net>
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

#ifndef __COREFOUNDATION_CFURLENUMERATOR__
#define __COREFOUNDATION_CFURLENUMERATOR__

#include <CoreFoundation/CFBase.h>
#include <CoreFoundation/CFURL.h>
#include <CoreFoundation/CFArray.h>
#include <CoreFoundation/CFError.h>
#include <Availability.h>

// CF_IMPLICIT_BRIDGING_ENABLED -- FIXME: this is supposed to be bridged
CF_EXTERN_C_BEGIN

typedef enum : CFIndex {
    kCFURLEnumeratorDefaultBehavior = 0,
    kCFURLEnumeratorDescendRecursively = 0x1,
    kCFURLEnumeratorSkipInvisibles = 0x2,
    kCFURLEnumeratorGenerateFileReferenceURLs = 0x4,
    kCFURLEnumeratorSkipPackageContents = 0x8,
    kCFURLEnumeratorIncludeDirectoriesPreOrder = 0x10,
    kCFURLEnumeratorIncludeDirectoriesPostOrder = 0x20,
    kCFURLEnumeratorGenerateRelativePathURLs = 0x40
} CFURLEnumeratorOptions;

typedef enum : CFIndex {
    kCFURLEnumeratorSuccess = 1,
    kCFURLEnumeratorEnd,
    kCFURLEnumeratorError,
    kCFURLEnumeratorDirectoryPostOrderSuccess
} CFURLEnumeratorResult;

typedef const struct __CFURLEnumerator* CFURLEnumeratorRef;

extern CFTypeID
CFURLEnumeratorGetTypeID();

extern CFURLEnumeratorRef
CFURLEnumeratorCreateForDirectoryURL(CFAllocatorRef alloc,
                                     CFURLRef directoryURL,
                                     CFURLEnumeratorOptions option,
                                     CFArrayRef propertyKeys);

extern CFURLEnumeratorRef
CFURLEnumeratorCreateForMountedVolumes(CFAllocatorRef alloc,
                                       CFURLEnumeratorOptions option,
                                       CFArrayRef propertyKeys);

extern CFIndex
CFURLEnumeratorGetDescendentLevel(CFURLEnumeratorRef enumerator);

extern void
CFURLEnumeratorSkipDescendents(CFURLEnumeratorRef enumerator);

extern CFURLEnumeratorResult
CFURLEnumeratorGetNextURL(CFURLEnumeratorRef enumerator,
                          CFURLRef*url,
                          CFErrorRef*error);

extern Boolean
CFURLEnumeratorGetSourceDidChange(CFURLEnumeratorRef enumerator)
    __API_DEPRECATED("Use File System Events instead", macos(10.6,10.7));

CF_EXTERN_C_END

#endif // __COREFOUNDATION_CFURLENUMERATOR__

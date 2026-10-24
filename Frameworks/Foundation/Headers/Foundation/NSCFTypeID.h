/* Copyright (c) 2009 Jens Ayton
   Copyright (C) 2024-2026 Zoe Knox <zoe@ravynsoft.com>
 
 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in all
 copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 SOFTWARE. */

/* IMPORTANT: these are for internal use only. They are subject to chage
   without notice. Use the appropriate functions (e.g. CFStringGetTypeID())
   rather than assuming specific values. Aligned to CF on 1/8/26.
*/

enum {
    kNSCFTypeNotAType = 0,
    kNSCFTypeTypeID,
    kNSCFTypeAllocator,
    kNSCFTypeBasicHash,
    kNSCFTypeBag,
    kNSCFTypeObject,
    /* empty slot 6 */
    kNSCFTypeString = 7, // "must be 0x7, now and forever"
    /* empty slots 8-15 */
    kNSCFTypeNull = 16,
    kNSCFTypeSet,
    kNSCFTypeDictionary,
    kNSCFTypeArray,
    kNSCFTypeData,
    kNSCFTypeBoolean,
    kNSCFTypeNumber,
    kNSCFTypeBinaryHeap,
    kNSCFTypeBitVector,
    kNSCFTypeCharacterSet,
    kNSCFTypeStorage,
    kNSCFTypeError,
    kNSCFTypeTree,
    kNSCFTypeURL,
    kNSCFTypeBundle,
    kNSCFTypePFactory,
    kNSCFTypePlugIn,
    kNSCFTypePlugInInstance,
    kNSCFTypeUUID,
    kNSCFTypeMessagePort,
    kNSCFTypeMachPort,
    kNSCFTypeStream,
    kNSCFTypeDate,
    kNSCFTypeRunLoop,
    kNSCFTypeRunLoopObserver,
    kNSCFTypeRunLoopSource,
    kNSCFTypeRunLoopTimer,
    kNSCFTypeTimeZone,
    kNSCFTypeCalendar,

    kNSCFTypeAttributedString,
    kNSCFTypeNotificationCenter,
    kNSCFTypeSocket
};

#if __OBJC__

@interface NSObject (CFTypeID)

- (unsigned)_cfTypeID;

@end

#endif

/*
 * Toll-free bridging glue
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

#include "CFBridging.h"
#include "CFInternal.h"
#include <Foundation/NSCFTypeID.h>

#define DEBUG 0
#if DEBUG
#define TRACE(...) printf(__VA_ARGS__)
#else
#define TRACE(...) /**/
#endif

NSRange NSMakeRange(NSUInteger location, NSUInteger length) {
   NSRange range = { location, length };
   return range;
}

CF_EXPORT int _CFRuntimeRegisterObjCClass(CFTypeID typeID, uintptr_t cls)
{
    if (typeID < 0 && typeID >= __CFRuntimeClassTableSize)
        return -1;
    TRACE("Registering ObjC class 0x%lx for type ID %d\n", cls, typeID);
    __CFRuntimeObjCClassTable[typeID] = cls;
    return 0;
}

CF_EXPORT void CFSetupFoundationBridging(void)
{
    TRACE("Setting up bridging\n");
    _CFRuntimeRegisterObjCClass(kNSCFTypeString, (uintptr_t)objc_lookUpClass("NSCFString"));
}

/*
 * Minimal boot framebuffer using EFI GOP
 *
 * Copyright (C) 2025-2026 Zoe Knox. All rights reserved.
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

#ifndef __IOG_SHIMS_H__
#define __IOG_SHIMS_H__

#include <IOKit/IOLib.h>

enum {
    DBG_IOG_TIMELOCK = 1,
    DBG_IOG_CAPTURED_RETRAIN,
    DBG_IOG_LOG_SYNCH,
    DBG_IOG_CONNECT_WORK_ASYNC,
    DBG_IOG_ASYNC_WORK,
    DBG_IOG_SYSTEM_WORK,
    DBG_IOG_MUX_ACTIVITY_CHANGE,
    DBG_IOG_MSG_CONNECT_CHANGE,
    DBG_IOG_EXT_PROCESS_CONNECT_CHANGE,
    DBG_IOG_EXT_END_CONNECT_CHANGE,
    DBG_IOG_SOURCE_SYSWORK_RESETCLAMSHELL_V2,
    DBG_IOG_SOURCE_SERVER_ACK_TIMEOUT,
    DBG_IOG_CONNECT_CHANGE_INTERRUPT_V2,
    DBG_IOG_SOURCE_VENDOR,
    DBG_IOG_SOURCE_SET_TRANSFORM,
    DBG_IOG_SOURCE_INIT_FB,
    DBG_IOG_FB_EXT_CLOSE,
    DBG_IOG_SOURCE_DO_SETUP,
    DBG_IOG_SOURCE_DO_SET_DISPLAY_MODE,
    DBG_IOG_BUILTIN_PANEL_POWER,
    DBG_IOG_SET_ATTR_FOR_CONN_EXT,
    DBG_IOG_SOURCE_SET_ATTR_FOR_CONN_EXT,
    DBG_IOG_SOURCE_OVERSCAN,
};

enum {
    ABL_SOURCE_IOD_ADDPARAMETERHANDLER = 1,
    ABL_SOURCE_IOD_SETPARAMETER,
    ABL_SET_BRIGHTNESS_PROBE,
    ABL_SET_BRIGHTNESS,
    ABL_COMMIT,
    ABL_SET_DISPLAY_POWER,
    ABL_SOURCE_IOD_DOINTEGERSET,
    ABL_SOURCE_IOD_DOUPDATE,
};

#define IOG_KTRACE_LOG_SYNCH(x)
#define GTRACE_LOG_SYNCH(x)

#define ABL_DO_UPDATE(...)
#define ABL_GT_SET_DISPLAY(...)
#define ABL_GT_DO_UPDATE(...)
#define ABL_GT_SET_DISPLAY_POWER(...)
#define ABL_GT_COMMITTED(...)
#define ABL_GT_SET_BRIGHTNESS(...)
#define ABL_GT_SET_BRIGHTNESS_PROBE(...)
#define ABLTRACERAW(...)
#define ABLTRACE_RAW(...)

inline void triggerEvent(int x) {
    IOLog("triggerEvent(%x) STUB!\n", x);
}

inline unsigned int clearEvent(int x) {
    IOLog("clearEvent(%x) STUB!\n", x);
    return 0;
}


#endif /* __IOG_SHIMS_H__ */


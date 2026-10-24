/**
 * log.h
 * Author: Zoe Knox      Created: 2026-7-3
 *
 * Copyright (C) 2026 Zoe Knox. All rights reserved.
 * SPDX: BSD-2-Clause
 *
 * This is an original implementation of Apple's libsystem_trace.dylib based on
 * open-source code, including ASL, XNU, Swift, and other sources, and the API
 * specs on developer.apple.com.
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

#ifndef __OS_LOG_H__
#define __OS_LOG_H__

#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>

#if __has_include_next(<os/log.h>)
#include_next <os/log.h>
#endif

typedef struct os_log_pack_s* os_log_pack_t;

extern struct os_log_s _os_log_disabled;
extern struct os_log_s _os_log_null;

__BEGIN_DECLS
/**
 * @function os_log_create
 * @abstract Creates a custom log object
 * @param subsystem  An identifier in reverse DNS notation
 * @param category   A category string within the subsystem
 * @return An os_log_t object to be passed to logging functions. The object
 *         is retained and should be released when done.
 */
__OSX_AVAILABLE_STARTING(__MAC_10_12, __IPHONE_10_0)
OS_EXPORT OS_WARN_RESULT OS_OBJECT_RETURNS_RETAINED
os_log_t OS_NOTHROW os_log_create(const char* subsystem, const char* category);

/**
 * @function os_log_type_enabled
 * @abstract Returns true if messages of `type` are enabled on `log`
 * @param log  The log object to query
 * @param type Type of log message to test if enabled
 */
__OSX_AVAILABLE_STARTING(__MAC_10_12, __IPHONE_10_0)
OS_EXPORT
bool os_log_type_enabled(os_log_t log, os_log_type_t type);

/**
 * @function os_log_info_enabled
 * @abstract Shortcut to check if OS_LOG_TYPE_INFO messages are enabled
 *           on `log`. See `os_log_type_enabled` for a generic interface.
 * @param log A log object
 */
__OSX_AVAILABLE_STARTING(__MAC_10_12, __IPHONE_10_0)
OS_EXPORT OS_NOTHROW
bool os_log_info_enabled(os_log_t log);

/**
 * @function os_log_debug_enabled
 * @abstract Shortcut to check if OS_LOG_TYPE_DEBUG messages are enabled 
 *           on `log`. See `os_log_type_enabled` for a generic interface.
 * @param log A log object
 */
__OSX_AVAILABLE_STARTING(__MAC_10_12, __IPHONE_10_0)
OS_EXPORT OS_NOTHROW
bool os_log_debug_enabled(os_log_t log);

/**
 * @function os_log_shim_enabled
 * @abstract Returns true if os_log logging is available for the specified
 *           log object and type of message. This is always true in ravynOS.
 * @param log  A log object
 * @param type Type of messages to check if enabled
 */
__OSX_AVAILABLE_STARTING(__MAC_10_12, __IPHONE_10_0)
OS_EXPORT OS_NOTHROW
bool os_log_shim_enabled(os_log_t log, os_log_type_t type);

__END_DECLS

#endif // __OS_LOG_H__

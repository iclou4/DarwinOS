/*
 * Apple-compatible message tracing
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

#ifndef _MSGTRACER_CLIENT_H
#define _MSGTRACER_CLIENT_H

#include <stdint.h>
#include <stdarg.h>
#include <msgtracer_keys.h>

__BEGIN_DECLS

/**
 * @function msgtracer_log_with_keys
 * @abstract Logs a trace message to the system log
 * @discussion Private API used for internal telemetry
 * @param domain A string representing the logging domain
 *               (e.g. client or framework name)
 * @param ... A variable number of key-value pairs given as
 *            `const char* key` and `const char* value`
 *            and terminated by a single NULL value
 * @result Returns zero on success, non-zero on error
 */

extern int32_t msgtracer_log_with_keys(const char* domain, ...);

/**
 * @function msgtracer_vlog_with_keys
 * @abstract Logs a trace message to the system log
 * @discussion Private API used for internal telemetry
 * @param domain A string representing the logging domain
 *               (e.g. client or framework name)
 * @param args A variable number of key-value pairs given as
 *             `const char* key` and `const char* value`
 * @result Returns zero on success, non-zero on error
 */
extern int32_t msgtracer_vlog_with_keys(const char* domain, va_list args);

__END_DECLS

#endif // _MSGTRACER_CLIENT_H

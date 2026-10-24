/*
 * Copyright (c) 2015-2016 Apple Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 *
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 *
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 *
 * @APPLE_LICENSE_HEADER_END@
 */
/* Modified by Zoe Knox for ravynOS July 2026. This note is in support of
 * clause 2.2 of the License
 */

#ifndef __os_log_private_h
#define __os_log_private_h

#include <os/log.h>
#include <firehose/tracepoint_private.h>
#include <sys/queue.h>

typedef struct {
    uint8_t  type;
    uint8_t  flags;
    uint16_t reserved;
    uint32_t payload_size;
} _os_log_arg_header;

typedef enum : uint8_t {
    OS_LOG_ARG_TYPE_SCALAR = 0,
    OS_LOG_ARG_TYPE_STRING = 1, // UTF-8
    OS_LOG_ARG_TYPE_WIDESTRING = 2, // 16 or 32 bit Unicode chars
    OS_LOG_ARG_TYPE_POINTER = 3,
    OS_LOG_ARG_TYPE_OBJECT = 4, // ObjC object
    OS_LOG_ARG_TYPE_ERRNO = 5,
    OS_LOG_ARG_TYPE_BOOLEAN = 6,
    OS_LOG_ARG_TYPE_DOUBLE = 7, // also for float
    OS_LOG_ARG_TYPE_INT = 8,
} _os_log_arg_type;

typedef enum : uint8_t {
    OS_LOG_FLAG_PUBLIC = 0,
    OS_LOG_FLAG_PRIVATE = 1,
    OS_LOG_FLAG_SENSITIVE = 2
} _os_log_arg_flags;

typedef enum : uint8_t {
    OS_LOG_MASK_NUMERIC = 0x10,
    OS_LOG_MASK_STRING = 0x20,
    OS_LOG_MASK_OBJECT = 0x40,
    OS_LOG_MASK_RESERVED = 0x80
} _os_log_arg_mask;


__BEGIN_DECLS

/*!
 * @function os_log_with_args
 *
 * @abstract
 * os_log variant that supports va_list args.
 *
 * @discussion
 * os_log variant that supports va_list args.  This SPI should only be used
 * to shim legacy logging systems through os_log.
 *
 * @param oslog
 * Pass OS_LOG_DEFAULT or a log object previously created with os_log_create.
 *
 * @param type
 * Pass one of the following message types.
 *   OS_LOG_TYPE_DEFAULT
 *   OS_LOG_TYPE_DEBUG
 *   OS_LOG_TYPE_INFO
 *   OS_LOG_TYPE_ERROR
 *   OS_LOG_TYPE_FAULT
 *
 * @param format
 * A format string to generate a human-readable log message when the log
 * line is decoded.  Supports all standard printf types in addition to %@
 * and %m (objects and errno respectively).
 *
 * @param args
 * A va_list containing the values for the format string.
 *
 * @param ret_addr
 * Pass the __builtin_return_address(0) of the function that created the
 * va_list from variadic arguments.  The caller must be the same binary
 * that generated the message and provided the format string.
 */
    __OSX_AVAILABLE_STARTING(__MAC_10_12, __IPHONE_10_0)
OS_EXPORT OS_NOTHROW OS_LOG_NOTAILCALL
void
os_log_with_args(os_log_t oslog, os_log_type_t type, const char *format, va_list args, void *ret_addr);

/*!
 * @enum oslog_stream_link_type_t
 */
OS_ENUM(oslog_stream_link_type, uint8_t,
    oslog_stream_link_type_log       = 0x0,
    oslog_stream_link_type_metadata  = 0x1,
    );

/*!
 * @typedef oslog_stream_buf_entry_t
 */
typedef struct oslog_stream_buf_entry_s {
	STAILQ_ENTRY(oslog_stream_buf_entry_s) buf_entries;
	uint64_t timestamp;
	int offset;
	uint16_t size;
	oslog_stream_link_type_t type;
	struct firehose_tracepoint_s metadata[];
} *oslog_stream_buf_entry_t;


#ifndef _STRUCT_TIMESPEC
struct timespec {
    time_t   tv_sec;        /* seconds */
    long     tv_nsec;       /* nanoseconds */
};
#endif

/*!
 * @typedef os_log_pack_t
 * AppleInternal. This type was guessed from clues in public documents
 */
typedef struct os_log_pack_s {
    uint64_t olp_continuous_time;
    struct timespec olp_wall_time;
    const void* olp_mh; // mach header of caller
    const void* olp_pc; // return addr of caller
    const char *olp_format;
    uint8_t olp_data[0];
} os_log_pack_s;
typedef os_log_pack_s *os_log_pack_t;

/* These private SPIs are implemented in libsystem_trace/log.c */
/**
 * @function os_log_encode
 * @param buffer        Pointer to where the encoded blob will be written
 * @param buffer_size   Capacity of the storage buffer in bytes
 * @param format        A printf-like format string
 * @param args          The args to fill in the format specifiers
 * @param flags         Privacy and formatting flags
 * @return The number of bytes written to `buffer`, or 0 if an error occurs
 */
__OSX_AVAILABLE_STARTING(__MAC_10_12, __IPHONE_10_0)
OS_EXPORT
size_t os_log_encode(void* buffer,
    size_t buffer_size,
    const char* format,
    va_list args,
    uint32_t flags); /*  __DARWIN_ALIAS(_os_log_encode); ?? */

/* Wrapper used by os/assumes.h */
size_t __os_log_encode(void* buffer,
    size_t buffer_size,
    const char* format,
    ...);

/**
 * @function os_log_pack_size
 * @abstract Computes and returns the number of bytes needed to store the
 *           formatted arguments and format string provided into an
 *           os_log_pack_t object.
 * @param format       printf-style format string
 * @param args         A binary buffer encoded by os_log_encode()
 * @param flags        Privacy and formatting flags
 * @return Returns the number of bytes needed for the log object
 */
__OSX_AVAILABLE_STARTING(__MAC_10_12, __IPHONE_10_0)
OS_EXPORT
size_t os_log_pack_size(const char* format, void* args, uint32_t flags);

/**
 * @function os_log_pack_fill
 * @abstract Adds metadata (timestamps, return address, mach_header) to the
 *           memory pointed to by pack, which has the capacity pack_size.
 *           The required size should be found by calling os_log_pack_size()
 *           first.
 * @param pack         Pointer to a os_log_pack_t buffer
 * @param pack_size    Size allocated to the pack
 * @param saved_errno  Contents of errno at the time of the call
 * @param dso          Return address of the caller
 * @param format       printf-style format string
 * @return Returns a pointer to the start of the payload area. The caller
 *         must copy the buffer from os_log_encode() to this address before
 *         calling os_log_pack_send(). Returns NULL on error.
 */
/* Internal ABI for 10.14+ */
__OSX_AVAILABLE_STARTING(__MAC_10_14, __IPHONE_12_0)
OS_EXPORT
uint8_t* os_log_pack_fill(void* pack,
    size_t pack_size,
    int saved_errno,
    const void* dso,
    const char* format);

/**
 * @function os_log_pack_send
 * @param pack A completed log pack object
 * @param log  Log object that will receive the pack
 * @param type Type of log message (debug, info, error, etc)
 *
 */
__OSX_AVAILABLE_STARTING(__MAC_10_12, __IPHONE_10_0)
OS_EXPORT
void os_log_pack_send(os_log_pack_t pack,
    os_log_t log,
    os_log_type_t type
);

/**
 * @function _os_log_pack_init
 * @param pack          Pointer to the output buffer
 * @param pack_size     Capacity of the output buffer in bytes
 * @param saved_errno   The value of errno at the time of the call
 * @param dso           Return address of the caller
 * @param format        A printf-like format string
 * @param args          The args to fill in the format specifiers
 * @return The number of bytes written to `buffer`, or 0 if an error occurs
 */
__OSX_AVAILABLE_STARTING(__MAC_10_12, __IPHONE_10_0)
OS_EXPORT
size_t _os_log_pack_init(void* pack,
    size_t pack_size,
    int saved_errno,
    const void* dso,
    const char* format,
    va_list args);


/**
 * @function __os_log_impl
 * @param dso       Return address of caller
 * @param log       Log object to receive the message
 * @param type      Type of log message (error, debug, etc)
 * @param format    A printf-like format string
 * @param buffer    Pointer to storage for the encoded args
 */
void __os_log_impl(void* dso,
    os_log_t log,
    os_log_type_t type,
    const char* format,
    uint8_t* buffer,
    uint32_t buffer_size) __DARWIN_ALIAS(_os_log_impl);

void __os_log_error_impl(void* dso,
    os_log_t log,
    const char* format,
    uint8_t* buffer,
    uint32_t buffer_size);

void __os_log_fault_impl(void* dso,
    os_log_t log,
    const char* format,
    uint8_t* buffer,
    uint32_t buffer_size);

size_t _os_trace_encode(void* buffer,
    size_t buffer_size,
    const char* format,
    va_list args,
    uint32_t flags);

void _os_trace_encode_and_send(void* dso,
    os_log_t log,
    os_log_type_t type,
    const char* format,
    va_list args,
    uint32_t flags);
void _os_trace_debug(const char* message);
void _os_trace_error(const char* message);

void _os_log_error(void* dso, os_log_t log, const char* format, ...);
void _os_log_fault(void* dso, os_log_t log, const char* format, ...);
void _os_log_info(void* dso, os_log_t log, const char* format, ...);
void _os_log_debug(void* dso, os_log_t log, const char* format, ...);

void _os_log_simple(void* dso,
    os_log_t log,
    os_log_type_t type,
    const char* message);

void _os_log_simple_impl(void* dso,
    os_log_t log,
    os_log_type_t type,
    const char* message);

void _os_log_set_mode(uint32_t mode);
uint32_t _os_log_get_mode(void);
void _os_log_preferences_refresh(void);


__END_DECLS

#endif // __os_log_private_h

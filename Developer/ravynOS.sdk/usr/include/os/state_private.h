/*
 * os/state_private.h
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
/* Original code based on public documents and WebKit source, and
 * a lot of hunches.
 */

#ifndef __OS_STATE_PRIVATE_H__
#define __OS_STATE_PRIVATE_H__

#include <os/base.h>
#include <sys/cdefs.h>
#include <stdint.h>
#include <dispatch/dispatch.h>

#define OS_STATE_DATA_SIZE_NEEDED(payload_size) \
    (sizeof(os_state_data_header_t) + (payload_size))

__BEGIN_DECLS

typedef uint64_t os_state_handle_t;

/* Completely fabricated header */
typedef struct os_state_data_header_s {
    uint64_t data_size;
    uint64_t data_type;
} os_state_data_header_t;

#define OS_STATE_HINT_NONE           0x00000000UL
#define OS_STATE_HINT_CRASH          0x00000001UL  /* State requested due to a process crash event */
#define OS_STATE_HINT_MEMORY         0x00000002UL  /* State requested due to memory pressure/jetsam event */
#define OS_STATE_HINT_PRESSURE       0x00000004UL  /* State requested due to general system/thermal pressure */
#define OS_STATE_HINT_TIMEOUT        0x00000008UL  /* State requested due to a watchdog or spin timeout */

/* FIXME: Are these the right values? I just guessed. */
enum : uint64_t {
	OS_STATE_DATA_SERIALIZED_NSCF_OBJECT = 1,
	OS_STATE_API_FAULT = 1,
	OS_STATE_API_TYPE_FAULT = OS_STATE_API_FAULT,
	OS_STATE_API_REQUEST = 2,
	OS_STATE_API_TYPE_REQUEST = OS_STATE_API_REQUEST,
};

struct os_state_hints_s {
    uint32_t osh_version;
    const char *osh_title;
    uint32_t osh_flags;
    uint32_t osh_resv;
    uint64_t osh_api;
};
typedef struct os_state_hints_s *os_state_hints_t;

struct os_state_data_s {
    uint32_t osd_version;
    uint32_t osd_type;
    uint64_t osd_size;
    char     osd_title[64];
    uint64_t osd_data_size;
    uint8_t  osd_data[];
};
typedef struct os_state_data_s *os_state_data_t;

#define OS_STATE_DATA_VERSION_1 1

/* 
 * Callback block for os_state_add_handler.
 * Returns a pointer to a structured os_state_data_s chunk upon system
 * diagnostic call requests.
 */
typedef os_state_data_t (^os_state_block_t)(os_state_hints_t hints);

/**
 * @function os_state_add_handler
 * @abstract Registers a handler to sample live state
 */
API_AVAILABLE(macos(10.12), ios(10.0), watchos(3.0), tvos(10.0))
OS_EXPORT OS_WARN_RESULT OS_NONNULL1 OS_NONNULL2
os_state_handle_t
os_state_add_handler(dispatch_queue_t queue, os_state_block_t block);

/**
 * @function os_state_remove_handler
 * @abstract Unregisters a state capture handler
 */
API_AVAILABLE(macos(10.12), ios(10.0), watchos(3.0), tvos(10.0))
OS_EXPORT
void
os_state_remove_handler(os_state_handle_t handle);

__END_DECLS

#endif /* __OS_STATE_PRIVATE_H__ */


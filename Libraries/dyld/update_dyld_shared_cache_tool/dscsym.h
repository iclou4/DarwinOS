int dscsym_save_dscsyms_for_current_caches(void);

#if 0
/*
 * Copyright (c) 2011-2026 Apple Inc. All rights reserved.
 * 
 * dscsym.h -- Private Apple header for handling Dyld Shared Cache (DSC) 
 *             symbol mapping, diagnostics, and stack logging layout.
 * AI slop generated from guesswork and public sources
 */

#ifndef _DSCSYM_H_
#define _DSCSYM_H_

#include <sys/types.h>
#include <stdint.h>
#include <mach/mach.h>
#include <uuid/uuid.h>

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// 1. MACROS & CONSTANTS
// =============================================================================

#define DSCSYM_MAGIC          0x44534353  /* 'DSCS' - Dyld Shared Cache Symbols Magic */
#define DSCSYM_VERSION_1      1
#define DSCSYM_VERSION_CURR   DSCSYM_VERSION_1

/* System storage paths for cache symbols */
#define DSCSYM_CACHE_DIR      "/var/db/dscsym"
#define DSCSYM_FILE_EXT       ".dscsym"

// =============================================================================
// 2. DATA STRUCTURES
// =============================================================================

/* Header layout at the absolute beginning of a .dscsym file */
struct dscsym_header {
    uint32_t    magic;           /* DSCSYM_MAGIC */
    uint32_t    version;         /* Target version (e.g., DSCSYM_VERSION_CURR) */
    uuid_t      cache_uuid;      /* The exact UUID of the corresponding Dyld Shared Cache */
    uint32_t    num_images;      /* Total number of dylibs indexed in this file */
    uint32_t    images_offset;   /* File offset to array of dscsym_image entries */
    uint32_t    symbols_offset;  /* File offset to the pooled symbol string table */
    uint32_t    symbols_size;    /* Size (in bytes) of the string table pool */
    uint32_t    flags;           /* Reserved for future or internal flags */
};

/* Describes a specific dylib image embedded inside the Shared Cache */
struct dscsym_image {
    uuid_t      image_uuid;      /* UUID of the specific dylib binary */
    uint64_t    text_va;         /* Target Virtual Address where __TEXT segment is mapped */
    uint32_t    text_size;       /* Total byte size of the __TEXT segment */
    uint32_t    path_offset;     /* Offset in the string table matching the runtime path */
    uint32_t    num_symbols;     /* Total number of local/exported symbols for this image */
    uint32_t    symbols_index;   /* Index into the global dscsym_symbol array */
};

/* Represents an individual symbol map entry */
struct dscsym_symbol {
    uint32_t    name_offset;     /* Offset into string table for the symbol name string */
    uint32_t    address_offset;  /* Offset from the image's text_va where this symbol starts */
};

// =============================================================================
// 3. PRIVATE FUNCTION PROTOTYPES (Diagnostic & Linker APIs)
// =============================================================================

/**
 * Automatically determines what dyld shared caches are currently loaded in the
 * kernel, iterates through them, builds their local symbol mappings, and saves
 * the resulting `.dscsym` files directly into the system symbol directory.
 * 
 * Invoked internally by `update_dyld_shared_cache` to clean up and sync the host's
 * `/var/db/dscsym/` storage during system updates or simulator cache updates.
 *
 * @return 0 on complete success, or a non-zero system error code on failure.
 */
int dscsym_save_dscsyms_for_current_caches(void);

/**
 * Validates and maps a .dscsym file into memory for parsing.
 * 
 * @param path The absolute filesystem path to the target .dscsym file.
 * @param out_header Pointer to receive the mapped file header context.
 * @return 0 on success, or standard POSIX error codes on failure.
 */
int dscsym_open_file(const char *path, struct dscsym_header **out_header);

/**
 * Unmaps or releases the resources allocated by dscsym_open_file.
 */
void dscsym_close_file(struct dscsym_header *header);

/**
 * Searches for an entry matching a specific Shared Cache by its UUID.
 * 
 * @param cache_uuid The unique identifier of the target dyld shared cache.
 * @param out_path Buffer to store the resulting file path if located.
 * @param max_path_len Max capacity of the out_path string buffer.
 * @return 1 if found, 0 if missing, or negative on system error.
 */
int dscsym_find_cache_by_uuid(const uuid_t cache_uuid, char *out_path, size_t max_path_len);

/**
 * Resolves a global virtual address in memory back to its text symbol, offset, and dylib path.
 * Primarily utilized by tools like spindump and crash logging daemons.
 * 
 * @param header Pointer to the mapped dscsym_header context.
 * @param va The target Virtual Address to look up.
 * @param out_image_path Buffer to hold the name/path of the containing dylib.
 * @param out_symbol_name Buffer to hold the resolved human-readable symbol name.
 * @param out_offset Byte distance between the requested VA and the symbol's base address.
 * @return 0 if resolution succeeded, or -1 if the address falls outside the cache boundaries.
 */
int dscsym_lookup_address(const struct dscsym_header *header, 
                          uint64_t va, 
                          char *out_image_path, 
                          char *out_symbol_name, 
                          uint64_t *out_offset);

/**
 * Generates or refreshes a .dscsym representation file from a raw dyld shared cache file.
 * Typically invoked by `update_dyld_shared_cache` during system updates.
 */
int dscsym_create_from_shared_cache(const char *shared_cache_path, const char *output_dscsym_path);

#ifdef __cplusplus
}
#endif

#endif /* _DSCSYM_H_ */
#endif // 0

/*
 * Copyright (c) 1999-2026 Apple Inc. All rights reserved.
 * 
 * Bom/Bom.h -- Private Apple header for manipulating Bill of Materials (.bom) files,
 *              primarily used by Installer.framework, mkbom, and lsbom.
 * AI slop generated from guesswork and public sources - 2026/07/28
 */

#ifndef _BOM_BOM_H_
#define _BOM_BOM_H_

#include <sys/types.h>
#include <stdint.h>
#include <sys/stat.h>

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// 1. MACROS & CONSTANTS
// =============================================================================

#define BOM_MAGIC              0x424f4d53  /* 'BOMS' - Bill Of Materials Storage */
#define BOM_VERSION_2          2
#define BOM_VERSION_CURR       BOM_VERSION_2

// =============================================================================
// 2. OPAQUE TYPES & CONTEXTS
// =============================================================================

typedef struct __BOMBom*       BOMBom;
typedef struct __BOMBomEnumerator* BOMBomEnumerator;
typedef struct __BOMFSObject*  BOMFSObject;

// =============================================================================
// 3. LIFECYCLE MANAGEMENT APIs
// =============================================================================

/**
 * Opens an existing .bom file from the filesystem for reading.
 * 
 * @param path Path to the target .bom file.
 * @return A valid BOMBom context, or NULL if the file cannot be read or parsed.
 */
BOMBom BOMBomOpen(const char *path);

/**
 * Creates a new, blank BOM tracking instance.
 * 
 * @param path Destination file path for the newly generated BOM.
 * @return A valid BOMBom instance configuration context.
 */
BOMBom BOMBomCreate(const char *path);

/**
 * Closes the BOM session and flushes/frees all associated memory structures.
 */
void BOMBomFree(BOMBom bom);

// =============================================================================
// 4. INVENTORY & ENUMERATION APIs
// =============================================================================

/**
 * Creates an iterator/enumerator to scan all entries sequentially inside a BOM.
 */
BOMBomEnumerator BOMBomEnumeratorNew(BOMBom bom);

/**
 * Frees an established enumerator session.
 */
void BOMBomEnumeratorFree(BOMBomEnumerator enumerator);

/**
 * Fetches the next system filesystem object in the BOM manifest.
 * 
 * @param enumerator Active enumerator context pointer.
 * @return The next BOMFSObject payload, or NULL if the end of manifest is reached.
 */
BOMFSObject BOMBomEnumeratorNext(BOMBomEnumerator enumerator);

// =============================================================================
// 5. FILESYSTEM OBJECT METADATA ACCESSORS
// =============================================================================

/**
 * Extract the relative or absolute installation target path of the given file entry.
 */
const char* BOMFSObjectPath(BOMFSObject fsObj);

/**
 * Gets standard Unix file descriptor status parameters (permissions, size, types).
 * 
 * @param fsObj Target metadata object pointer.
 * @param outStat Pointer to a standard POSIX stat struct to hold structural details.
 */
void BOMFSObjectGetStat(BOMFSObject fsObj, struct stat *outStat);

/**
 * Retrieves the stored cyclic redundancy checksum for the target data file.
 * Used by verification tools to confirm structural package integrity.
 */
uint32_t BOMFSObjectChecksum(BOMFSObject fsObj);

/**
 * Returns the architecture payload type if the entry represents a Mach-O binary 
 * (e.g., universal, x86_64, arm64).
 */
const char* BOMFSObjectArchitecture(BOMFSObject fsObj);

// =============================================================================
// 6. MUTATOR APIs (Creation Side)
// =============================================================================

/**
 * Bundles a raw file entry descriptor explicitly into an active write BOM generation block.
 */
int BOMBomInsertFSObject(BOMBom bom, BOMFSObject fsObj);

#ifdef __cplusplus
}
#endif

#endif /* _BOM_BOM_H_ */


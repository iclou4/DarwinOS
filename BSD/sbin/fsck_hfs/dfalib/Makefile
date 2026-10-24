SUBDIR = 
MK_WERROR = no

LIB = dfa
SRCS =  BTree.c \
        BTreeAllocate.c \
        BTreeMiscOps.c \
        BTreeNodeOps.c \
        BTreeScanner.c \
        BTreeTreeOps.c \
        BlockCache.c \
        CatalogCheck.c \
        DecompMakeData.c \
        HardLinkCheck.c \
        SAllocate.c \
        SBTree.c \
        SCatalog.c \
        SControl.c \
        SDevice.c \
        SExtents.c \
        SKeyCompare.c \
        SRebuildBTree.c \
        SRepair.c \
        SStubs.c \
        SUtils.c \
        SVerify1.c \
        SVerify2.c \
        VolumeBitmapCheck.c \
        dirhardlink.c \
        fsck_journal.c \
        hfs_endian.c \
        uuid.c
MAN = 

LIBUTIL = ${ROOT_SOURCE_DIR}/BSD/lib/libutil
IOKIT_STORAGE = ${ROOT_SOURCE_DIR}/Kernel/Extensions/IOStorageFamily
CFLAGS = --sysroot=${RAVYN_SDKROOT} -mmacos-version-min=${MACOS_VERSION_MIN} \
        -DBSD -DDEBUG_BUILD \
        -I${.CURDIR} -I${.CURDIR}/include -I${.CURDIR:H}/../mount_hfs/include \
        -I${LIBUTIL} -I${IOKIT_STORAGE}
LDFLAGS = --sysroot=${RAVYN_SDKROOT} -mmacos-version-min=${MACOS_VERSION_MIN} \
        -fuse-ld=${LD} -L${ROOT_BINARY_DIR}${LIBUTIL} -lutil

.include <bsd.lib.mk>

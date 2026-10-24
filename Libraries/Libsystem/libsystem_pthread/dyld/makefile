.PATH: ${.CURDIR:H}

LIB = pthread
SUBDIR =

.include "common.mk"

CFLAGS := ${CFLAGS:S/${.CURDIR}/${.CURDIR:H}/} -DVARIANT_STATIC -DVARIANT_DYLD
ACFLAGS := ${CFLAGS}

.include <bsd.lib.mk>

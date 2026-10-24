#	$OpenBSD: Makefile,v 1.4 2005/05/16 15:22:46 espie Exp $

MK_WERROR = no
SUBDIR =
PROG=	patch
SRCS=	patch.c pch.c inp.c util.c backupfile.c mkpath.c

all: to-sysroot

to-sysroot: ${PROG}
	mkdir -p ${SYSROOT_DIR}/bin
	cp -f ${PROG} ${SYSROOT_DIR}/bin/

.include <bsd.prog.mk>


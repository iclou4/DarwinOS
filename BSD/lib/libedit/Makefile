SUBDIR =
MK_WERROR = no
LIB = edit

.PATH: ${.CURDIR}/src
SRCS =  chared.c \
        chartype.c \
        common.c \
        el.c \
        eln.c \
        emacs.c \
        filecomplete.c \
        getline.c \
        hist.c \
        history.c \
        historyn.c \
        keymacro.c \
        literal.c \
        map.c \
        parse.c \
        prompt.c \
        read.c \
        readline.c \
        reallocarr.c \
        refresh.c \
        search.c \
        sig.c \
        strlcat.c \
        strlcpy.c \
        terminal.c \
        tokenizer.c \
        tokenizern.c \
        tty.c \
        unvis.c \
        vi.c \
        vis.c \
        wcsdup.c \
        vi.h \
        emacs.h \
        common.h \
        func.h \
        fcns.h \
        help.h

CFLAGS = --sysroot=${RAVYN_SDKROOT} -mmacos-version-min=${MACOS_VERSION_MIN} \
	-I${.CURDIR}/src -I${.CURDIR} -I${.OBJDIR}
LDFLAGS = --sysroot=${RAVYN_SDKROOT} -mmacos-version-min=${MACOS_VERSION_MIN} \
        -fuse-ld=${LD}

AHDR = vi.h emacs.h common.h 
ASRC = $(.CURDIR)/src/vi.c $(.CURDIR)/src/emacs.c $(.CURDIR)/src/common.c

all: lib${LIB}.dylib

.for f in emacs vi common
${f}.h: ${.CURDIR}/src/${f}.c
	AWK=awk sh ${.CURDIR}/src/makelist -h ${.ALLSRC} > ${.TARGET} 
.endfor

fcns.h: ${AHDR}
	AWK=awk sh ${.CURDIR}/src/makelist -fh ${.ALLSRC} > ${.TARGET}

func.h: ${AHDR}
	AWK=awk sh ${.CURDIR}/src/makelist -fc ${.ALLSRC} > ${.TARGET}

help.h: ${ASRC}
	AWK=awk sh ${.CURDIR}/src/makelist -bh ${.ALLSRC} > ${.TARGET}

lib${LIB}.dylib: lib${LIB}.a
	${CC} ${LDFLAGS} -shared -o ${.TARGET} \
	  -Wl,-install_name,/usr/lib/libedit.dylib \
	  -Wl,-current_version,3.1 -Wl,-compatibility_version,1.0 \
	  -Wl,-force_load,${.OBJDIR}/lib${LIB}.a -lncurses
	cp -f ${.OBJDIR}/lib${LIB}.dylib ${RAVYN_SDKROOT}/usr/lib/
	cp -f ${.OBJDIR}/lib${LIB}.dylib ${SYSROOT_DIR}/usr/lib/

.include <bsd.lib.mk>


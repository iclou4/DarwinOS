.include <src.opts.mk>

SUBDIR =
MK_WERROR=no
CFLAGS+= -mmacos-version-min=${MACOS_VERSION_MIN} --sysroot=${RAVYN_SDKROOT} \
	-B${TOOLS} -fuse-ld=${LD} -I${ROOT_SOURCE_DIR}/Libraries/zlib \
	-I${ROOT_SOURCE_DIR}/BSD/lib/libpng

all: libfreetype.a
libfreetype.a:
	cmake -S ${.CURDIR} -B ${.OBJDIR} \
		-DCMAKE_C_FLAGS="${CFLAGS}" \
		-DCMAKE_C_COMPILER=${CC} \
		-DCMAKE_CXX_COMPILER=${CXX} \
		-DCMAKE_SYSTEM_NAME=Darwin \
		-DCMAKE_SYSROOT=${RAVYN_SDKROOT} \
		-DCMAKE_BUILD_TYPE=Release \
		-DCMAKE_INSTALL_PREFIX=/usr \
		-DBUILD_SHARED_LIBS=OFF \
		-DFT_DISABLE_BZIP2=TRUE \
		-DFT_DISABLE_HARFBUZZ=TRUE \
		-DFT_DISABLE_BROTLI=TRUE \
		-DSKIP_INSTALL_ALL=TRUE \
		-DZLIB_FOUND=TRUE -DPNG_FOUND=TRUE \
		-DZLIB_LIBRARIES="-lz" \
		-DPNG_LIBRARY="-L${.OBJDIR:H}/png -lpng" \
		-DPNG_PNG_INCLUDE_DIR="${.OBJDIR:H}/png"
	${MAKE} -C ${.OBJDIR}

install:
includes:
clean:

.include <bsd.lib.mk>


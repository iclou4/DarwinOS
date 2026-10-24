
all: ${.OBJDIR}/lib/libncurses.6.dylib

${.OBJDIR}/lib/libncurses.6.dylib: ${.CURDIR}/Makefile
	CC=${CC} CXX=${CXX} \
	CFLAGS="--sysroot=${RAVYN_SDKROOT} -mmacos-version-min=${MACOS_VERSION_MIN}" \
	CXXFLAGS="--sysroot=${RAVYN_SDKROOT} -mmacos-version-min=${MACOS_VERSION_MIN}" \
	LDFLAGS="--sysroot=${RAVYN_SDKROOT} -mmacos-version-min=${MACOS_VERSION_MIN} -fuse-ld=${LD}" \
	${.CURDIR}/configure --prefix=/usr \
        --sysconfdir=/private/etc \
        --localstatedir=/private/var \
        --target=${CpuArch}-apple-darwin19 \
        --host=${CpuArch}-apple-darwin19 \
	--with-strip-program=${TOOLS}/strip \
        --without-ada --without-tests --enable-pc-files \
        --with-shared --with-normal --with-debug \
        --enable-rpath --without-dlsym --disable-lib-suffixes \
        --enable-termcap --enable-symlinks --enable-ext-colors \
        --enable-rgb-color --with-pthread --enable-pthreads-eintr \
        --enable-reentrant
	MAKEFLAGS="" ${TOOLS}/gmake
	MAKEFLAGS="" ${TOOLS}/gmake install DESTDIR=${RAVYN_SDKROOT}
	MAKEFLAGS="" ${TOOLS}/gmake install DESTDIR=${SYSROOT_DIR}
.for prog in clear infocmp tabs tic toe tput tset
	ln -sf ${CpuArch}-apple-darwin19-${prog} ${SYSROOT_DIR}/usr/bin/${prog}
.endfor

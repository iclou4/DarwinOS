#!/usr/bin/env zsh
SRCROOT=${CIRRUS_WORKING_DIR:-$PWD}
PLATFORM=${PLATFORM:-$(uname -m).$(uname -p)}
OSNAME=$(uname -s)
PREFIX=${PREFIX:-/usr}
if [ "x${OSNAME}" = "xLinux" ]; then
    HW_CPUS=$(grep -c ^processor /proc/cpuinfo)
else
    HW_CPUS=$(sysctl -n hw.ncpu)
fi
CORES=${CORES:-${HW_CPUS}}
BUILDROOT=/usr/obj/${SRCROOT}/${PLATFORM}
MAKESYSPATH=${PWD}/share/mk
if [ "x${OSNAME}" = "xFreeBSD" ]; then
  MAKE=${MAKE:-make}
  BMAKE=make
  GMAKE=gmake
else
  MAKE=${MAKE:-bmake}
  BMAKE=bmake
  GMAKE=make
fi
__MAKE_CONF=$PWD/tools/make.conf
export MAKE BMAKE GMAKE
export PREFIX __MAKE_CONF SRCROOT PLATFORM MAKESYSPATH

PATH=${OBJTOP:-/usr/obj/ravynOS/${PLATFORM}}/tmp/obj-tools/usr/bin:${PATH}
TERM=vt100
export PATH TERM

install() {
}


base_build() {
    if [ $clean -eq 1 ]; then
	cd ${BUILDROOT}
	for d in $(echo *|sed -E 's/(sys|CoreServices|Frameworks|SysApps)//g'); do
	    logdate "Cleaning $d"
	    rm -rf "${BUILDROOT}/${d}"
	done
    fi
    cd ${SRCROOT}
    ${MAKE} -j${CORES} buildworld
    if [ $? -ne 0 ]; then exit $?; fi
}

kernel_build() {
    cd ${SRCROOT}
    if [ $clean -eq 1 ]; then
	logdate "Cleaning object dir"
	rm -rf ${OBJTOP}/Kernel/xnu
    fi
    ${MAKE} -j${CORES} buildkernel
    if [ $? -ne 0 ]; then exit $?; fi
}

kext_build() {
    cd ${SRCROOT}
    if [ $clean -eq 1 ]; then
      logdate "Cleaning object dir"
      rm -rf ${OBJTOP}/Kernel/Extensions
    fi
    ${MAKE} -j${CORES} kext
    if [ $? -ne 0 ]; then exit $?; fi
}

kernelcache() {
    ${MAKE} prelink
    if [ $? -ne 0 ]; then exit $?; fi
}

system_build() {
    if [ $clean -eq 1 ]; then
	logdate "Cleaning object dir"
	rm -rf ${BUILDROOT}/Frameworks ${BUILDROOT}/CoreServices \
		${BUILDROOT}/SysApps
    fi
    cd ${SRCROOT}
    echo "CIRRUS_CI=${CIRRUS_CI}"
    if [ "x${CIRRUS_CI}" = "xtrue" ]; then
	echo "Symlinking OBJTOP/usr/include/machine for CI"
        ln -sf ${SRCROOT}/sys/$(uname -m)/include \
	/usr/obj/${SRCROOT}/${PLATFORM}/tmp/usr/include/machine
    fi
    ${MAKE} -f Makefile.ravynOS prep
    if [ $? -ne 0 ]; then exit $?; fi
    cp -fv share/mk/* /usr/share/mk/
    ${MAKE} -f Makefile.ravynOS
    if [ $? -ne 0 ]; then exit $?; fi
}

extras_build() {
    if [ $clean -eq 1 ]; then
	logdate "Cleaning object dir"
	rm -rf ${BUILDROOT}/release/dist/ravynOS
    fi
    cd ${SRCROOT}
    if [ ! -d neofetch ]; then
        git clone https://github.com/ravynsoft/neofetch.git
    fi
    mkdir -p ${BUILDROOT}/release/dist/ravynOS/usr/bin/
    mkdir -p ${BUILDROOT}/release/dist/ravynOS/usr/share/man/man1/
    cp -fv ${SRCROOT}/neofetch/neofetch \
	${BUILDROOT}/release/dist/ravynOS/usr/bin/
    cp -fv ${SRCROOT}/neofetch/neofetch.1 \
	${BUILDROOT}/release/dist/ravynOS/usr/share/man/man1/
    chmod 755 ${BUILDROOT}/release/dist/ravynOS/usr/bin/neofetch
    if [ $? -ne 0 ]; then exit $?; fi
    cd ${SRCROOT}
    if [ ! -d plutil ]; then
        git clone https://github.com/ravynsoft/plutil.git
    fi
    cd plutil
    git submodule update --init --recursive
    PATH=${PATH}:${TMPBIN} \
	CMAKE_FLAGS="-DLIBXML2_LIBRARY=${BUILDROOT}/tmp/usr/lib/libxml2.so -DLIBXML2_INCLUDE_DIR=${BUILDROOT}/tmp/usr/include/libxml2" \
	${TMPLEGACY}/gmake
    PATH=${PATH}:${TMPBIN} ${TMPLEGACY}/gmake \
	DESTDIR=${BUILDROOT}/release/dist/ravynOS install
    if [ $? -ne 0 ]; then exit $?; fi
    cd ${SRCROOT}
}

systempkg() {
    rm -f dist/ravynOS.txz
    tar cvJ -C /usr/obj/${SRCROOT}/${PLATFORM}/release/dist/ravynOS --gid 0 --uid 0 -f $(pwd)/dist/ravynOS.txz .
    if [ $? -ne 0 ]; then exit $?; fi
}

isoalt() {
    cp -fv version.txt ISO/overlays/ramdisk/version
    mkdir -p /usr/local/furybsd/$(uname -m)/cache/$(head -1 version.txt)/base
    cp -fv /usr/obj/${SRCROOT}/${PLATFORM}/release/base.txz \
      /usr/obj/${SRCROOT}/${PLATFORM}/release/kernel.txz \
      ${SRCROOT}/dist/ravynOS.txz \
      /usr/local/furybsd/$(uname -m)/cache/$(head -1 version.txt)/base/
    cd ISO; RAVYNOS=${SRCROOT} ./build.sh
}

cleandir() {
    logdate "Cleaning object dir"
    rm -rf "${BUILDROOT}"
}

logdate() {
  echo -n ':: '
  if ! [ "z$1" = "z" ]; then
      echo -n "$1 "
  fi
  echo $(date '+%c %z')
}

set_options() {
    if [ $log -eq 1 ]; then
        if [ $preserve -eq 0 ]; then
            exec > >(tee ${logfile}) 2>&1
        else
            exec > >(tee -a ${logfile}) 2>&1
        fi
    fi
    echo ":: ravynOS Build Tool [Prefix ${PREFIX} Cores ${CORES} Platform ${PLATFORM}]"
    echo ":: Host OS [${OSNAME}]"
    logdate "Starting"
}

usage() {
    echo ""
    echo "Usage: " $(basename $0) " [-n] [-p] target [target] ..."
    echo "    -n         No Log - do not create a log file"
    echo "    -p         Preserve Log - append to existing file"
    echo "    -c         Clean target before build"
    echo "    --purge    Purge entire object dir before build"
    echo ""
    echo "Targets:"
    echo "    base kernel kext kernelcache system extras systempkg"
    echo "    iso isoalt install all"
    echo ""
}

preserve=0
log=1
purge=0
clean=0
logfile="build.log"
set -A targets

while ! [ "z$1" = "z" ]; do
    case "$1" in
        -n) log=0 ;;
        -p) preserve=1 ;;
	      --purge) purge=1 ;;
	      -c) clean=1 ;;
        -*) ;;
        *) targets+=("$1") ;;
    esac
    shift
done
set_options

if [ $purge -eq 1 ]; then
    logdate "Purging object directory ${BUILDROOT}"
    rm -rf ${BUILDROOT}
fi

if [ ${#targets} -eq 0 ]; then
    if [ $purge -eq 0 ]; then
      	usage
      	exit 1
    fi
fi

set -- $targets[@]
while ! [ "z$1" = "z" ]; do
    arg=$(echo $1|sed -e 's/[ ]*//g') # trim whitespace
    case "$arg" in
        base) base_build ;;
        kernel) kernel_build ;;
	      kext) kext_build ;;
	      kernelcache) kernelcache ;;
        system) system_build ;;
        extras) extras_build ;;
        iso) iso_build ;;
        systempkg) systempkg ;;
        isoalt) isoalt ;;
	      install) install ;;
        all) kernel_build; base_build; \
             system_build; extras_build; iso_build ;;
        *) echo target is $arg
    esac
    shift
done
logdate "Finished"

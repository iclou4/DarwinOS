# ------------------------------------------------------------------------
#  RAVYNOS BUILD SYSTEM - Developer Platform
# ------------------------------------------------------------------------

PLATFORM_DIR = ${ROOT_BINARY_DIR}/Developer/Platforms/ravynOS.platform

all: ${PLATFORM_DIR} Info.plist

${PLATFORM_DIR}:
	mkdir -p ${PLATFORM_DIR}

Info.plist: ${PLATFORM_DIR} ${.CURDIR}/Info.plist
	cp -f ${.CURDIR}/Info.plist ${PLATFORM_DIR}/

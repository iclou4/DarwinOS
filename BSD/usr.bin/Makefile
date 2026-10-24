MK_TESTS = no
CFLAGS = --sysroot=${RAVYN_SDKROOT} -mmacos-version-min=${MACOS_VERSION_MIN} \
        -Wno-nullability-completeness
LDFLAGS = --sysroot=${RAVYN_SDKROOT} -mmacos-version-min=${MACOS_VERSION_MIN} \
        -fuse-ld=${LD}
.export CFLAGS LDFLAGS MK_TESTS

SUBDIR = apply basename chroot cut date dirname env false find getopt grep 
SUBDIR += head kill killall patch pfetch printf sed sw_vers 
SUBDIR += tail tee time touch true uname users whereis which xargs yes
#SUBDIR += fetch id sort who

.include <bsd.subdir.mk>

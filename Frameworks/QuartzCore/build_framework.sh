#!/usr/bin/env bash
set -e

TOOLCHAIN="/Library/Developer/Toolchains/Default.xctoolchain"
export PATH="$TOOLCHAIN/usr/bin:/usr/local/bin:/usr/bin:/bin"
export LD_LIBRARY_PATH="$TOOLCHAIN/usr/lib:$LD_LIBRARY_PATH"
export DEVELOPER_DIR="/Developer"

ROOT_SOURCE_DIR="/home/antonov/projects/ravynos"
ROOT_BINARY_DIR="/home/antonov/projects/build"
RAVYN_SDKROOT="$ROOT_BINARY_DIR/Developer/Platforms/ravynOS.platform/Developer/SDKs/ravynOS.sdk"
SYSROOT_DIR="$ROOT_BINARY_DIR/sysroot"
DEVEL="$ROOT_SOURCE_DIR/Developer"
TOOLS="$TOOLCHAIN/usr/bin"
SRCTOP="$ROOT_SOURCE_DIR"
OBJTOP="$ROOT_BINARY_DIR"
DSYMUTIL="$TOOLS/dsymutil"

CC="$TOOLS/clang"
CXX="$TOOLS/clang++"
LD="$TOOLS/ld64"
AR="$TOOLS/llvm-ar --format=darwin"
CpuArch="x86_64"
MACHINE_CPUARCH="x86_64"
BuildArch="X86"
MACOS_VERSION_MIN="10.15"
MACOSX_DEPLOYMENT_TARGET="10.15"

export ROOT_SOURCE_DIR ROOT_BINARY_DIR RAVYN_SDKROOT SYSROOT_DIR DEVEL TOOLCHAIN TOOLS SRCTOP OBJTOP DSYMUTIL
export CC CXX LD AR CpuArch MACHINE_CPUARCH BuildArch MACOS_VERSION_MIN MACOSX_DEPLOYMENT_TARGET
export MAKESYSPATH="$ROOT_SOURCE_DIR/BSD/share/mk"

cd "$ROOT_SOURCE_DIR/Frameworks/QuartzCore"
echo "=== Building QuartzCore.framework ==="
bmake -m "$MAKESYSPATH" all
echo "=== Installing QuartzCore.framework ==="
bmake -m "$MAKESYSPATH" install

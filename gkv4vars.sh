#!/bin/bash

if [ -z "${SYSROOT}" ]; then
    SYSROOT=$HOME/opt/gkv4
fi

if [ -z "${TOOLSDIR}" ]; then
    TOOLSDIR=$HOME/opt/gkv4-build
fi

echo "Using GK toolchain in $TOOLSDIR.  Sysroot set to $SYSROOT."

# check for presence of g++
if [ ! -x $TOOLSDIR/bin/arm-none-gkos-c++ ]; then
    echo "Toolchain not found.  Please run ./build-toolchain.sh first."
    exit -1
fi

PATH="$TOOLSDIR/bin:$PATH"

export SYSROOT
export TOOLSDIR
export PATH

# cmake options
SCRIPT_DIR="$(dirname "$(readlink -f "${BASH_SOURCE[0]}")")"
CMAKE_OPTS="-DCMAKE_TOOLCHAIN_FILE=$SCRIPT_DIR/toolchain-gkos.cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo -DCMAKE_INSTALL_PREFIX=$SYSROOT/usr"

export CMAKE_OPTS

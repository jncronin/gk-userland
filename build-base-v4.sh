#!/bin/bash

set -e

# check TOOLSDIR, SYSROOT and gcc set
if [[ -z "$TOOLSDIR" ]]; then
    echo "TOOLSDIR not set.  Please source ./gkv4vars.sh first"
    exit 0
fi

if [[ -z "$SYSROOT" ]]; then
    echo "SYSROOT not set.  Please source ./gkv4vars.sh first"
    exit 0
fi

if [[ ! -x $TOOLSDIR/bin/aarch64-none-gkos-c++ ]]; then
    echo "Toolchain not found.  Please source ./gkv4vars.sh first"
    exit 0
fi

if [[ -z "$CMAKE_OPTS" ]]; then
    echo "CMAKE_OPTS not set.  Please source ./gkv4vars.sh first"
    exit 0
fi

# gk-menu
cmake $CMAKE_OPTS -S gk-menu -B build-v4/gk-menu
make -j16 -C build-v4/gk-menu
cd build-v4/gk-menu && cpack && cd ../..

# gk-supervisor
cmake $CMAKE_OPTS -S gk-supervisor -B build-v4/gk-supervisor
make -j16 -C build-v4/gk-supervisor
cd build-v4/gk-supervisor && cpack && cd ../..

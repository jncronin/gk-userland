#!/bin/sh

set -e

if [ -z "${SYSROOT}" ]; then
    SYSROOT=$HOME/opt/gk
fi

if [ -z "${TOOLSDIR}" ]; then
    TOOLSDIR=$HOME/opt/gk-build
fi

echo "Building GK toolchain in $TOOLSDIR.  Sysroot set to $SYSROOT."

mkdir -p $SYSROOT/usr

# copy newlib headers
cp -dpR newlib-cygwin/newlib/libc/include $SYSROOT/usr

# copy gk headers
cp -dpR gloss-gk/gk-userlandinterface/* $SYSROOT/usr/include
cp -dpR gloss-gk/inc-gk/* $SYSROOT/usr/include

# build binutils
mkdir -p build/binutils
cd build/binutils
../../binutils-gdb/configure --target=arm-none-gkos --prefix="$TOOLSDIR" --with-sysroot="$SYSROOT" --disable-werror --disable-gdb
make -j16 all
make -j16 install
cd ../..

export PATH="$TOOLSDIR/bin:$PATH"

# build gcc
mkdir -p build/gcc
cd build/gcc
../../gcc/configure --target=arm-none-gkos --prefix="$TOOLSDIR" --with-sysroot="$SYSROOT" --enable-languages=c,c++
make -j16 all-gcc
make -j16 install-gcc
cd ../..

# build newlib
mkdir -p build/newlib
cd build/newlib
../../newlib-cygwin/configure --target=arm-none-gkos --disable-multilib --enable-static --disable-shared --prefix=/usr --disable-newlib-supplied-syscalls --enable-newlib-io-long-long --enable-newlib-io-c99-formats --enable-newlib-mb --enable-newlib-reent-check-verify --enable-newlib-register-fini --enable-newlib-retargetable-locking --enable-newlib-reent-thread-local
make -j16 all-target-newlib
DESTDIR=$SYSROOT make -j16 install-target-newlib
cp -dpR $SYSROOT/usr/arm-none-gkos/* $SYSROOT/usr
cd ../..

# build gk-gloss
cmake -DCMAKE_TOOLCHAIN_FILE=../toolchain-gkos.cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo -DCMAKE_INSTALL_PREFIX=$SYSROOT/usr -S gloss-gk -B build/gloss-gk
make -C build/gloss-gk -j16 install

# build newlib start files
cd build/newlib
make configure-target-libgloss
cd arm-none-gkos/libgloss
make arm/crt0.o
cp arm/crt0.o $SYSROOT/usr/lib
cd ../../../..

# build libgcc
cd build/gcc
make -j16 all-target-libgcc
make -j16 install-target-libgcc

# build libstdc++
make -j16 all-target-libstdc++-v3
make -j16 install-target-libstdc++-v3
cd ../..

# move target files from toolsdir to sysroot
cp -dpR $TOOLSDIR/arm-none-gkos/include/* $SYSROOT/usr/include
cp -dpR $TOOLSDIR/arm-none-gkos/lib/* $SYSROOT/usr/lib

# report success
echo "Successfully built gkos toolchain in $TOOLSDIR"

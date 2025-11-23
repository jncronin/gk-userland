#!/bin/sh

set -e

if [ -z "${SYSROOT}" ]; then
    SYSROOT=$HOME/opt/gkv4
fi

if [ -z "${TOOLSDIR}" ]; then
    TOOLSDIR=$HOME/opt/gkv4-build
fi

echo "Building GK toolchain in $TOOLSDIR.  Sysroot set to $SYSROOT."

mkdir -p $SYSROOT/usr

# copy newlib headers
cp -dpR newlib-cygwin/newlib/libc/include $SYSROOT/usr
rm $SYSROOT/usr/include/threads.h

# copy gk headers
cp -dpR gloss-gk/gk-userlandinterface/* $SYSROOT/usr/include
cp -dpR gloss-gk/inc-gk/* $SYSROOT/usr/include

# build binutils
mkdir -p build-v4/binutils
cd build-v4/binutils
../../binutils-gdb/configure --target=aarch64-none-gkos --prefix="$TOOLSDIR" --with-sysroot="$SYSROOT" --disable-werror --disable-gdb
make -j16 all
make -j16 install
cd ../..

export PATH="$TOOLSDIR/bin:$PATH"

# build gcc
mkdir -p build-v4/gcc
cd build-v4/gcc
../../gcc/configure --target=aarch64-none-gkos --prefix="$TOOLSDIR" --with-sysroot="$SYSROOT" --enable-languages=c,c++ --enable-threads=posix
make -j16 all-gcc
make -j16 install-gcc
cd ../..

# build newlib
mkdir -p build-v4/newlib
cd build-v4/newlib
../../newlib-cygwin/configure --target=aarch64-none-gkos --disable-multilib --enable-static --enable-shared --prefix=/usr --disable-newlib-supplied-syscalls --enable-newlib-io-long-long --enable-newlib-io-c99-formats --enable-newlib-mb --enable-newlib-reent-check-verify --enable-newlib-register-fini --enable-newlib-retargetable-locking --enable-newlib-reent-thread-local
make -j16 all-target-newlib
DESTDIR=$SYSROOT make -j16 install-target-newlib
cp -dpR $SYSROOT/usr/aarch64-none-gkos/* $SYSROOT/usr
cd ../..

# build gk-gloss
cmake -DCMAKE_TOOLCHAIN_FILE=../toolchain-gkosv4-static.cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo -DCMAKE_INSTALL_PREFIX=$SYSROOT/usr -S gloss-gk -B build-v4/gloss-gk
make -C build-v4/gloss-gk -j16 install

# build start file
aarch64-none-gkos-gcc -c -o build-v4/gloss-gk/crt0.o gloss-gk/src-gloss/crt0v4.S
cp -dp build-v4/gloss-gk/crt0.o $SYSROOT/usr/lib

# build libgcc
cd build-v4/gcc
make -j16 all-target-libgcc
make -j16 install-target-libgcc
cd ../..

# build libstdc++ - use separate build tree (even though it builds gcc again) so that we pull in new newlib defines
mkdir -p build-v4/libstdc++
cd build-v4/libstdc++
../../gcc/configure --target=aarch64-none-gkos --prefix="$TOOLSDIR" --with-sysroot="$SYSROOT" --enable-languages=c,c++ --enable-threads=posix
make -j16 all-target-libstdc++-v3
make -j16 install-target-libstdc++-v3
cd ../..

# move target files from toolsdir to sysroot
cp -dpR $TOOLSDIR/aarch64-none-gkos/include/* $SYSROOT/usr/include
cp -dpR $TOOLSDIR/aarch64-none-gkos/lib/* $SYSROOT/usr/lib

# now make a shared version of libgloss
cmake -DCMAKE_TOOLCHAIN_FILE=../toolchain-gkosv4.cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo -DCMAKE_INSTALL_PREFIX=$SYSROOT/usr -S gloss-gk -B build-v4/gloss-gk-shared
make -C build-v4/gloss-gk-shared -j16 install

# report success
echo "Successfully built gkos toolchain in $TOOLSDIR"

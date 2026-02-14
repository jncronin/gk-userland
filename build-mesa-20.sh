#!/bin/sh

# LLVM native tablegen
cmake -DCMAKE_BUILD_TYPE=Release -DLLVM_BUILD_RUNTIME=OFF -DLLVM_INCLUDE_TESTS=OFF -DLLVM_INCLUDE_EXAMPLES=OFF -DLLVM_ENABLE_BACKTRACES=OFF -DLLVM_INCLUDE_BENCHMARKS=OFF -DLLVM_TARGETS_TO_BUILD="AArch64" -DLLVM_ENABLE_PROJECTS="" -DLLVM_ENABLE_PIC=OFF -DLLVM_ENABLE_BINDINGS=OFF -DLLVM_BUILD_TOOLS=OFF -DLLVM_BUILD_UTILS=OFF -DLLVM_BUILD_RUNTIMES=OFF -DCMAKE_INSTALL_PREFIX=llvm-buildsys-tablegen -S llvm-project-14.0.6.src/llvm -B build-llvm-buildsys-tablegen

cmake --build build-llvm-buildsys-tablegen/ --target llvm-tblgen -j16

# Hosted LLVM using build system tablegen
cmake $CMAKE_OPTS -DCMAKE_BUILD_TYPE=Release -DLLVM_BUILD_RUNTIME=OFF -DLLVM_INCLUDE_TESTS=OFF -DLLVM_INCLUDE_EXAMPLES=OFF -DLLVM_ENABLE_BACKTRACES=OFF -DLLVM_INCLUDE_BENCHMARKS=OFF -DLLVM_TARGETS_TO_BUILD="AArch64" -DLLVM_ENABLE_PROJECTS="" -DLLVM_ENABLE_PIC=OFF -DLLVM_ENABLE_BINDINGS=OFF -DLLVM_BUILD_TOOLS=OFF -DLLVM_BUILD_UTILS=OFF -DLLVM_BUILD_RUNTIMES=OFF -DLLVM_BUILD_STATIC=ON -DUNIX=ON -DLLVM_TABLEGEN=/home/jncronin/src/build-llvm-buildsys-tablegen/bin/llvm-tblgen -DCMAKE_CROSSCOMPILING=ON -DLLVM_DEFAULT_TARGET_TRIPLE=aarch64-none-gkos -DLLVM_HOST_TRIPLE=aarch64-none-gkos -DLLVM_TARGET_ARCH=AArch64 -DCMAKE_INSTALL_PREFIX=mesa-20.3.5/subprojects/llvm -S llvm-project-14.0.6.src/llvm -B build-llvm

make -j16 -C build-llvm install-llvm-libraries install-llvm-headers install-cmake-exports

# mesa

meson setup -Dgallium-drivers=swrast -Dvulkan-drivers= -Dplatforms= -Dglx=disabled -Dosmesa=gallium -Dllvm=enabled -Dshared-glapi=disabled -Degl=disabled -Ddri3=disabled -Ddefault_library=static -Ddri-drivers= --cross-file cross_file.txt -Dbuildtype=release -Db_ndebug=true -Dcmake_prefix_path=~/opt/gkv4/usr/lib/cmake --prefix ~/opt/gkv4/usr builddir

meson compile -C builddir
meson install -C builddir

cp ~/opt/gkv4/usr/lib/libOSMesa.a ~/opt/gkv4/usr/lib/libGL.a

#!/bin/sh

cmake -DCMAKE_TOOLCHAIN_FILE=../toolchain-m4.cmake -DCMAKE_INSTALL_PREFIX=~/src/gk -S gloss-gk -B build/gloss-gk
make -C build/gloss-gk install

cmake -DCMAKE_TOOLCHAIN_FILE=../toolchain-m4.cmake -DCMAKE_INSTALL_PREFIX=~/src/gk -S zlib-1.3 -B build/zlib
make -C build/zlib install

cmake -DCMAKE_TOOLCHAIN_FILE=../toolchain-m4.cmake -DCMAKE_INSTALL_PREFIX=~/src/gk -DCMAKE_FIND_ROOT_PATH=~/src/gk -S libpng-1.6.40 -B build/libpng
make -C build/libpng install


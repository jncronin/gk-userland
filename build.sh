#!/bin/bash

set -e

cmake -DCMAKE_TOOLCHAIN_FILE=../toolchain-m4.cmake -DCMAKE_INSTALL_PREFIX=~/src/gk -S gloss-gk -B build/gloss-gk
make -C build/gloss-gk install

cmake -DCMAKE_TOOLCHAIN_FILE=../toolchain-m4.cmake -DCMAKE_INSTALL_PREFIX=~/src/gk -S zlib-1.3 -B build/zlib
make -C build/zlib install

cmake -DCMAKE_TOOLCHAIN_FILE=../toolchain-m4.cmake -DCMAKE_INSTALL_PREFIX=~/src/gk -DCMAKE_FIND_ROOT_PATH=~/src/gk -S libpng-1.6.40 -B build/libpng
make -C build/libpng install

cmake -DCMAKE_TOOLCHAIN_FILE=../toolchain-m4.cmake -DCMAKE_INSTALL_PREFIX=~/src/gk -DCMAKE_FIND_ROOT_PATH=~/src/gk -S libiconv-cmake-master/ -B build/libiconv
make -C build/libiconv install

cmake -DCMAKE_TOOLCHAIN_FILE=../toolchain-m4.cmake -DCMAKE_INSTALL_PREFIX=~/src/gk -DCMAKE_FIND_ROOT_PATH=~/src/gk -DUNIX=ON -DSDL_LIBC=ON -DSDL_PTHREADS=ON -DSDL_THREADS=ON -DSDL_OPENGL=OFF -DSDL_OPENGLES=OFF -S SDL2-2.28.5/ -B build/sdl2
make -C build/sdl2 install
ln -s ~/src/gk/lib/liblibiconv.a ~/src/gk/lib/libiconv.a

cmake -DCMAKE_TOOLCHAIN_FILE=../toolchain-m4.cmake -DCMAKE_INSTALL_PREFIX=~/src/gk -DCMAKE_FIND_ROOT_PATH=~/src/gk -S tinygl-main/ -B build/tinygl
make -C build/tinygl install

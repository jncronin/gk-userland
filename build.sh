#!/bin/bash

set -e

mkdir -p ~/src/gk/include/sys
touch ~/src/gk/include/sys/gk.h

cmake -DCMAKE_TOOLCHAIN_FILE=../toolchain-m7.cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo -DCMAKE_INSTALL_PREFIX=~/src/gk -S gloss-gk -B build/gloss-gk
make -C build/gloss-gk install

cmake -DCMAKE_TOOLCHAIN_FILE=../toolchain-m7.cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=~/src/gk -DCMAKE_FIND_ROOT_PATH=~/src/gk -S libiconv-cmake-master/ -B build/libiconv
make -C build/libiconv install
rm -rf ~/src/gk/lib/libiconv.a
ln -s ~/src/gk/lib/liblibiconv.a ~/src/gk/lib/libiconv.a

cmake -DCMAKE_TOOLCHAIN_FILE=../toolchain-m7.cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=~/src/gk -S zlib-1.3 -B build/zlib
make -C build/zlib install

cmake -DCMAKE_TOOLCHAIN_FILE=../toolchain-m7.cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=~/src/gk -DCMAKE_FIND_ROOT_PATH=~/src/gk -S libpng-1.6.40 -B build/libpng
make -C build/libpng install

cmake -DCMAKE_TOOLCHAIN_FILE=../toolchain-m7.cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=~/src/gk -DCMAKE_FIND_ROOT_PATH=~/src/gk -S rain-1-s-shell/ -B build/s
make -C build/s install

cmake -DCMAKE_TOOLCHAIN_FILE=../toolchain-m7.cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=~/src/gk -DCMAKE_FIND_ROOT_PATH=~/src/gk -S tools/ -B build/tools
make -C build/tools install

# strip bin files for gk
arm-none-eabi-strip -S ~/src/gk/bin/ls
arm-none-eabi-strip -S ~/src/gk/bin/tftpd
arm-none-eabi-strip -S ~/src/gk/bin/sh
arm-none-eabi-strip -S ~/src/gk/bin/echo

tar -cf gk-`date +"%s"`.tar -C ~/src/gk bin/ls bin/tftpd bin/sh bin/echo

#cd busybox-1.36.1
#CFLAGS="-mthumb -mcpu=cortex-m4 -mfloat-abi=hard -mfpu=fpv5-sp-d16 -ffast-math --specs=nano.specs -ffunction-sections -fdata-sections -ffreestanding -D__GAMEKID__ -D_POSIX_THREADS -I/home/jncronin/src/gk/include" LDFLAGS="-Wl,--entry,_mainCRTStartup -Wl,--section-start,.init=0 -Wl,-Ttext,0x32 -Wl,-z,max-page-size=32 -Wl,--gc-sections -L/home/jncronin/src/gk/lib -lm -Wl,--whole-archive /home/jncronin/src/gk/lib/libgloss-gk.a -Wl,--no-whole-archive -Wl,-q" CROSS_COMPILE="arm-none-eabi-" make

cmake -DCMAKE_TOOLCHAIN_FILE=../toolchain-m7.cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=~/src/gk -DCMAKE_FIND_ROOT_PATH=~/src/gk -DTINYGL_BUILD_EXAMPLES=OFF -S tinygl-main/ -B build/tinygl
make -C build/tinygl install
mkdir -p ~/src/gk/include/GL
cp ~/src/gk/include/TGL/* ~/src/gk/include/GL

cmake -DCMAKE_TOOLCHAIN_FILE=../toolchain-m7.cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo -DCMAKE_INSTALL_PREFIX=~/src/gk -DCMAKE_FIND_ROOT_PATH=~/src/gk -DUNIX=ON -DSDL_LIBC=ON -DSDL_PTHREADS=ON -DSDL_THREADS=ON -DSDL_OPENGL=OFF -DSDL_OPENGLES=OFF -S SDL2-2.28.5/ -B build/sdl2
make -C build/sdl2 install

cmake -DCMAKE_TOOLCHAIN_FILE=../toolchain-m7.cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=~/src/gk -DCMAKE_FIND_ROOT_PATH=~/src/gk -DUNIX=ON jpeg-9f/ -B build/jpeg
make -C build/jpeg install

cmake -DCMAKE_TOOLCHAIN_FILE=../toolchain-m7.cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=~/src/gk -DCMAKE_FIND_ROOT_PATH=~/src/gk -DUNIX=ON SDL_image-release-2.8.2/ -B build/sdl2_image
make -C build/sdl2_image install

cmake -DCMAKE_TOOLCHAIN_FILE=../toolchain-m7.cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=~/src/gk -DCMAKE_FIND_ROOT_PATH=~/src/gk -DUNIX=ON freetype-2.13.2/ -B build/freetype2
make -C build/freetype2 install

cmake -DCMAKE_TOOLCHAIN_FILE=../toolchain-m7.cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=~/src/gk -DCMAKE_FIND_ROOT_PATH=~/src/gk -DUNIX=ON -DSDL2TTF_SAMPLES=OFF SDL_ttf-release-2.22.0/ -B build/sdl2_ttf
make -C build/sdl2_ttf install

cmake -DCMAKE_TOOLCHAIN_FILE=../toolchain-m7.cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo -DCMAKE_INSTALL_PREFIX=~/src/gk -DCMAKE_FIND_ROOT_PATH=~/src/gk -S SDL-1.2-main/ -B build/sdl12
make -C build/sdl12 install



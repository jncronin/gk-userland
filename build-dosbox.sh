#!/bin/sh


PKG_CONFIG_PATH=/home/jncronin/opt/gk/usr/lib/pkgconfig SDL_CONFIG=/home/jncronin/opt/gk/usr/bin/sdl-config CC=arm-none-gkos-gcc CXX=arm-none-gkos-g++ ../dosbox-0.74-3/configure --host=arm-none-gkos --prefix=/home/jncronin/src/dosbox --disable-sdltest  --disable-opengl --disable-alsatest --disable-alsa-midi --with-sdl-prefix=/home/jncronin/opt/gk/usr --with-sdl-exec-prefix=/home/jncronin/opt/gk/usr --enable-dynrec --enable-dynamic-core PKG_CONFIG_PATH=/home/jncronin/opt/gk/usr/lib/pkgconfig 

sed -i 's/C_TARGETCPU UNKNOWN/C_TARGETCPU ARMV4LE/;s~/\* #undef C_CORE_INLINE \*/~#define C_CORE_INLINE 1~;s~/\* #undef C_UNALIGNED_MEMORY \*/~#define C_UNALIGNED_MEMORY 1~;s~/\* #undef C_DYNREC \*/~#define C_DYNREC 1~' config.h


# v4
PKG_CONFIG_PATH=/home/jncronin/opt/gkv4/usr/lib/pkgconfig SDL_CONFIG=/home/jncronin/opt/gkv4/usr/bin/sdl-config CC=aarch64-none-gkos-gcc CXX=aarch64-none-gkos-g++ ../dosbox-0.74-3/configure --host=aarch64-none-gkos --prefix=/home/jncronin/src/dosboxv4 --disable-sdltest  --disable-opengl --disable-alsatest --disable-alsa-midi --with-sdl-prefix=/home/jncronin/opt/gkv4/usr --with-sdl-exec-prefix=/home/jncronin/opt/gkv4/usr --enable-dynrec --enable-dynamic-core PKG_CONFIG_PATH=/home/jncronin/opt/gkv4/usr/lib/pkgconfig 
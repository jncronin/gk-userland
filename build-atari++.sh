#! /bin/sh

# build in tree
CC=arm-none-gkos-gcc CXX=arm-none-gkos-g++ ./configure --enable-X11=no --enable-SDL=yes --enable-CURSES=no --host=arm-none-gkos --prefix=/home/jncronin/src/atari++-gk --with-sdl-prefix=/home/jncronin/opt/gk/usr

# for v4:
CC=aarch64-none-gkos-gcc CXX=aarch64-none-gkos-g++ ./configure --enable-X11=no --enable-SDL=yes --enable-CURSES=no --host=aarch64-none-gkos --prefix=/home/jncronin/src/atari++-gkv4 --with-sdl-prefix=/home/jncronin/opt/gkv4/usr

# edit types.h after configure to remove rpl_malloc define and change HAVE_MALLOC to 1, re-add defines for having SDL

CC=aarch64-none-gkos-gcc CXX=aarch64-none-gkos-g++ make -j16

# running, e.g.: ./atari++ -cartpath ~/jncronin/Downloads/atari800/River\ Raid.rom -carttype 8k -acceptlicence true -width 320 -height 240 -frontend SDL -sound SDL

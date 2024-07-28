#!/bin/sh

mkdir -p build/tcl8
cd build/tcl8
CFLAGS="-mthumb -mcpu=cortex-m7 -mfloat-abi=hard -mfpu=fpv5-d16 -ffast-math -include sys/gk.h -ffunction-sections -fdata-sections -ffreestanding -D__GAMEKID__ -D_POSIX_THREADS=1 -DHAVE_STRLCPY  -Dunix -DUNIX -D_POSIX_MONOTONIC_CLOCK -D_POSIX_TIMERS=1 -D_POSIX_READER_WRITER_LOCKS=1 -I/home/jncronin/src/gk/include -O3" CXXFLAGS="-fnothreadsafe-statics -Wno-psabi -mthumb -mcpu=cortex-m7 -mfloat-abi=hard -mfpu=fpv5-d16 -ffast-math -include sys/gk.h -ffunction-sections -fdata-sections -ffreestanding -D__GAMEKID__ -D_POSIX_THREADS=1  -Dunix -DUNIX -D_POSIX_MONOTONIC_CLOCK -D_POSIX_TIMERS=1 -D_POSIX_READER_WRITER_LOCKS=1 -I/home/jncronin/src/gk/include -O3" LDFLAGS="-Wl,--entry,_mainCRTStartup -Wl,--section-start,.init=0 -Wl,-Ttext,0x32 -Wl,-z,max-page-size=32 -Wl,--gc-sections -L/home/jncronin/src/gk/lib -lm -Wl,--whole-archive -llibcharset /home/jncronin/src/gk/lib/libgloss-gk.a /home/jncronin/src/gk/lib/libgk.a -Wl,--no-whole-archive -Wl,-q" SHELL=/bin/bash ../../tcl8.6.14/unix/configure --host=arm-none-eabi --enable-static --disable-shared --disable-load --disable-unload --prefix=/home/jncronin/src/gk
make -j16 libtcl8.6.a
make -j16 libtclstub8.6.a
make install-headers
cp libtcl8.6.a /home/jncronin/src/gk/lib
cp libtclstub8.6.a /home/jncronin/src/gk/lib
cp tclConfig.sh /home/jncronin/src/gk/lib
cp ../../tcl8.6.14/unix/tclooConfig.sh /home/jncronin/src/gk/lib
mkdir -p /home/jncronin/src/gk/lib/pkgconfig
cp tcl.pc /home/jncronin/src/gk/lib/pkgconfig

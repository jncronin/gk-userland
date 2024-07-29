#!/bin/sh
# CFLAGS_FOR_TARGET="-mthumb -mcpu=cortex-m7 -mfloat-abi=hard -mfpu=fpv5-d16 -ffast-math -include sys/gk.h -ffunction-sections -fdata-sections -ffreestanding -D__GAMEKID__ -D_POSIX_THREADS=1 -DHAVE_STRLCPY  -Dunix -DUNIX -D_POSIX_MONOTONIC_CLOCK -D_POSIX_TIMERS=1 -D_POSIX_READER_WRITER_LOCKS=1 -I/home/jncronin/src/gk/include -O3" CXXFLAGS_FOR_TARGET="-fnothreadsafe-statics -Wno-psabi -mthumb -mcpu=cortex-m7 -mfloat-abi=hard -mfpu=fpv5-d16 -ffast-math -include sys/gk.h -ffunction-sections -fdata-sections -ffreestanding -D__GAMEKID__ -D_POSIX_THREADS=1  -Dunix -DUNIX -D_POSIX_MONOTONIC_CLOCK -D_POSIX_TIMERS=1 -D_POSIX_READER_WRITER_LOCKS=1 -I/home/jncronin/src/gk/include -O3" SHELL=/bin/bash 

CFLAGS_FOR_TARGET="-D__GAMEKID__" ../../newlib-cygwin/configure --target=arm-none-eabi --enable-static --disable-shared --prefix=/home/jncronin/src/gk --disable-newlib-supplied-syscalls --enable-newlib-io-long-long --enable-newlib-io-c99-formats --enable-newlib-mb --enable-newlib-reent-check-verify --enable-newlib-register-fini --enable-newlib-retargetable-locking

make -j16 all-target-newlib
make -j16 install-target-newlib

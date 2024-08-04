#!/bin/sh

# needs texinfo bison yacc mpfr mpc gmp

export PATH="/home/jncronin/src/gk-build/bin:$PATH"

mkdir -p build/binutils
cd build/binutils
../../binutils-gdb/configure --disable-shared --disable-nls --enable-initfini-array --without-x --disable-gdbtk --without-tcl --without-tk --enable-plugins --disable-gdb --without-gdb --with-sysroot=/home/jncronin/src/gc --prefix=/home/jncronin/src/gk-build --target=arm-none-eabi
make -j16 install

cd ../..

mkdir -p build/gcc
cd build/gcc
../../gcc/configure --disable-shared --disable-nls --enable-checking=release --enable-languages=c,c++ --with-newlib --with-gnu-as --with-gnu-ld --with-native-system-header-dir=/include --target=arm-none-eabi --with-multilib-list=@t-gk --with-sysroot=/home/jncronin/src/gk --prefix=/home/jncronin/src/gk-build
make -j16 all-gcc
make -j16 install-gcc

CPPFLAGS="-D__GAMEKID__ -include sys/gk.h -isystem /home/jncronin/src/gk/include -isystem /home/jncronin/src/gk/arm-none-eabi/include" CFLAGS="-ftls-model=local-exec -mthumb -mcpu=cortex-m7 -mfloat-abi=hard -mfpu=fpv5-d16 -ffast-math -ffunction-sections -fdata-sections -ffreestanding" CXXFLAGS="-ftls-model=local-exec -mthumb -mcpu=cortex-m7 -mfloat-abi=hard -mfpu=fpv5-d16 -ffast-math -ffunction-sections -fdata-sections -ffreestanding" LDFLAGS="-L/home/jncronin/src/gk/arm-none-eabi/lib -L/home/jncronin/src/gk/lib -Wl,--whole-archive -llibcharset -lgloss-gk -lgk -Wl,--no-whole-archive" PATH="/home/jncronin/src/gk-build/bin:$PATH" make -j16 all-target-libgcc
CPPFLAGS="-D__GAMEKID__ -include sys/gk.h -isystem /home/jncronin/src/gk/include -isystem /home/jncronin/src/gk/arm-none-eabi/include" CFLAGS="-ftls-model=local-exec -mthumb -mcpu=cortex-m7 -mfloat-abi=hard -mfpu=fpv5-d16 -ffast-math -ffunction-sections -fdata-sections -ffreestanding" CXXFLAGS="-ftls-model=local-exec -mthumb -mcpu=cortex-m7 -mfloat-abi=hard -mfpu=fpv5-d16 -ffast-math -ffunction-sections -fdata-sections -ffreestanding" LDFLAGS="-L/home/jncronin/src/gk/arm-none-eabi/lib -L/home/jncronin/src/gk/lib -Wl,--whole-archive -llibcharset -lgloss-gk -lgk -Wl,--no-whole-archive" PATH="/home/jncronin/src/gk-build/bin:$PATH" make -j16 install-target-libgcc
#CPPFLAGS="-D__GAMEKID__ -include sys/gk.h -isystem /home/jncronin/src/gk/include -isystem /home/jncronin/src/gk/arm-none-eabi/include" CFLAGS="-ftls-model=local-exec -mthumb -mcpu=cortex-m7 -mfloat-abi=hard -mfpu=fpv5-d16 -ffast-math -ffunction-sections -fdata-sections -ffreestanding" CXXFLAGS="-ftls-model=local-exec -mthumb -mcpu=cortex-m7 -mfloat-abi=hard -mfpu=fpv5-d16 -ffast-math -ffunction-sections -fdata-sections -ffreestanding" LDFLAGS="-L/home/jncronin/src/gk/arm-none-eabi/lib -L/home/jncronin/src/gk/lib -Wl,--whole-archive -llibcharset -lgloss-gk -lgk -Wl,--no-whole-archive" PATH="/home/jncronin/src/gk-build/bin:$PATH" make -j16 all-target-libstdc++

cd ../..

mkdir -p build/libstdc++
cd build/libstdc++

CPPFLAGS="-D__GAMEKID__ -include sys/gk.h -isystem /home/jncronin/src/gk/include -isystem /home/jncronin/src/gk/arm-none-eabi/include" CFLAGS="-ftls-model=local-exec -mthumb -mcpu=cortex-m7 -mfloat-abi=hard -mfpu=fpv5-d16 -ffast-math -ffunction-sections -fdata-sections" CXXFLAGS="-ftls-model=local-exec -mthumb -mcpu=cortex-m7 -mfloat-abi=hard -mfpu=fpv5-d16 -ffast-math -ffunction-sections -fdata-sections" LDFLAGS="-L/home/jncronin/src/gk/arm-none-eabi/lib -L/home/jncronin/src/gk/lib -Wl,--whole-archive -llibcharset -lgloss-gk -lgk -Wl,--no-whole-archive" PATH="/home/jncronin/src/gk-build/bin:$PATH" ../../gcc/libstdc++-v3/configure --host=arm-none-eabi --disable-multilib --disable-nls --disable-libstdcxx-pch --disable-shared --with-newlib --prefix=/home/jncronin/src/gk
make -j16 install

cd ../..

#!/bin/bash

set -e

mkdir -p ~/src/gk/include/sys
touch ~/src/gk/include/sys/gk.h

export GK_ROOT="$HOME/src/gk"
export PATH="$HOME/src/gk-build/bin:$PATH"

GK_CPPFLAGS="-D__GAMEKID__ -include sys/gk.h -isystem $GK_ROOT/include -isystem $GK_ROOT/arm-none-eabi/include"
GK_CPPFLAGS_CXX="-isystem $GK_ROOT/include/c++/13.3.1 -isystem $GK_ROOT/include/c++/13.3.1/arm-none-eabi $GK_CPPFLAGS"
GK_CXX_CFLAGS="-ftls-model=local-exec -mthumb -mcpu=cortex-m7 -mfloat-abi=hard -mfpu=fpv5-d16 -ffast-math -ffunction-sections -fdata-sections"
GK_CFLAGS="$GK_CXX_CFLAGS"
GK_CXXFLAGS="$GK_CXX_CFLAGS -Wno-psabi"
GK_LDFLAGS="-Wl,--entry,_mainCRTStartup -Wl,--section-start,.init=0 -Wl,-Ttext,0x32 -Wl,-z,max-page-size=32 -Wl,--gc-sections -L$GK_ROOT/arm-none-eabi/lib -L$GK_ROOT/lib -Wl,--whole-archive -llibcharset $GK_ROOT/lib/libgloss-gk.a $GK_ROOT/lib/libgk.a -Wl,--no-whole-archive -Wl,-q"


mkdir -p build/newlib
cd build/newlib
CFLAGS_FOR_TARGET="-ftls-model=local-exec -mthumb -mcpu=cortex-m7 -mfloat-abi=hard -mfpu=fpv5-d16 -ffast-math -include sys/gk.h -ffunction-sections -fdata-sections -ffreestanding -D__GAMEKID__ -D_POSIX_THREADS=1 -DHAVE_STRLCPY  -Dunix -DUNIX -D_POSIX_MONOTONIC_CLOCK -D_POSIX_TIMERS=1 -D_POSIX_READER_WRITER_LOCKS=1 -I/home/jncronin/src/gk/include -O2 -g" CXXFLAGS_FOR_TARGET="-ftls-model=local-exec -fnothreadsafe-statics -Wno-psabi -mthumb -mcpu=cortex-m7 -mfloat-abi=hard -mfpu=fpv5-d16 -ffast-math -include sys/gk.h -ffunction-sections -fdata-sections -ffreestanding -D__GAMEKID__ -D_POSIX_THREADS=1  -Dunix -DUNIX -D_POSIX_MONOTONIC_CLOCK -D_POSIX_TIMERS=1 -D_POSIX_READER_WRITER_LOCKS=1 -I/home/jncronin/src/gk/include -O3" SHELL=/bin/bash  ../../newlib-cygwin/configure --target=arm-none-eabi --disable-multilib --enable-static --disable-shared --prefix=/home/jncronin/src/gk --disable-newlib-supplied-syscalls --enable-newlib-io-long-long --enable-newlib-io-c99-formats --enable-newlib-mb --enable-newlib-reent-check-verify --enable-newlib-register-fini --enable-newlib-retargetable-locking --enable-newlib-reent-thread-local
make -j16 all-target-newlib
make -j16 install-target-newlib
make -j16 all-target-libgloss
make -j16 install-target-libgloss
cp ~/src/gk/arm-none-eabi/lib/crt0.o ~/src/gk/lib
cd ../..

cmake -DCMAKE_TOOLCHAIN_FILE=../toolchain-m7.cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo -DCMAKE_INSTALL_PREFIX=~/src/gk -S gloss-gk -B build/gloss-gk
make -C build/gloss-gk -j16 install

cmake -DCMAKE_TOOLCHAIN_FILE=../toolchain-m7.cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=~/src/gk -DCMAKE_FIND_ROOT_PATH=~/src/gk -S libiconv-cmake-master/ -B build/libiconv
make -C build/libiconv -j16 install
rm -rf ~/src/gk/lib/libiconv.a
ln -s ~/src/gk/lib/liblibiconv.a ~/src/gk/lib/libiconv.a

cmake -DCMAKE_TOOLCHAIN_FILE=../toolchain-m7.cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=~/src/gk -S zlib-1.3 -B build/zlib
make -C build/zlib -j16 install
cp ~/src/gk/lib/libzlib.a ~/src/gk/lib/libz.a

cmake -DCMAKE_TOOLCHAIN_FILE=../toolchain-m7.cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=~/src/gk -DCMAKE_FIND_ROOT_PATH=~/src/gk -S libpng-1.6.40 -B build/libpng
make -C build/libpng -j16 install
cp ~/src/gk/lib/liblibpng16.a ~/src/gk/lib/libpng16.a

cmake -DCMAKE_TOOLCHAIN_FILE=../toolchain-m7.cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=~/src/gk -DCMAKE_FIND_ROOT_PATH=~/src/gk -S rain-1-s-shell/ -B build/s
make -C build/s -j16 install

cmake -DCMAKE_TOOLCHAIN_FILE=../toolchain-m7.cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=~/src/gk -DCMAKE_FIND_ROOT_PATH=~/src/gk -S tools/ -B build/tools
make -C build/tools -j16 install

# strip bin files for gk
arm-none-eabi-strip -S ~/src/gk/bin/ls
arm-none-eabi-strip -S ~/src/gk/bin/tftpd
arm-none-eabi-strip -S ~/src/gk/bin/sh
arm-none-eabi-strip -S ~/src/gk/bin/echo

#tar -cf gk-`date +"%s"`.tar -C ~/src/gk bin/ls bin/tftpd bin/sh bin/echo

#cd busybox-1.36.1
#CFLAGS="-mthumb -mcpu=cortex-m4 -mfloat-abi=hard -mfpu=fpv5-sp-d16 -ffast-math --specs=nano.specs -ffunction-sections -fdata-sections -ffreestanding -D__GAMEKID__ -D_POSIX_THREADS -I/home/jncronin/src/gk/include" LDFLAGS="-Wl,--entry,_mainCRTStartup -Wl,--section-start,.init=0 -Wl,-Ttext,0x32 -Wl,-z,max-page-size=32 -Wl,--gc-sections -L/home/jncronin/src/gk/lib -lm -Wl,--whole-archive /home/jncronin/src/gk/lib/libgloss-gk.a -Wl,--no-whole-archive -Wl,-q" CROSS_COMPILE="arm-none-eabi-" make

cmake -DCMAKE_TOOLCHAIN_FILE=../toolchain-m7.cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=~/src/gk -DCMAKE_FIND_ROOT_PATH=~/src/gk -DTINYGL_BUILD_EXAMPLES=OFF -S tinygl-main/ -B build/tinygl
make -C build/tinygl -j16 install
mkdir -p ~/src/gk/include/GL
#cp ~/src/gk/include/TGL/* ~/src/gk/include/GL

mkdir -p build/mesa4
cd build/mesa4
CPPFLAGS="$GK_CPPFLAGS" CFLAGS="$GK_CFLAGS" CXXFLAGS="$GK_CXXFLAGS" LDFLAGS="$GK_LDFLAGS" ../../Mesa-4.0.3/configure --host=arm-none-eabi --enable-static --disable-shared --prefix=/home/jncronin/src/gk
cd src && make -j16 install && cd ..
cd include && make install && cd ..
cd src-glu && make install && cd ..
cd ../..

cmake -DCMAKE_TOOLCHAIN_FILE=../toolchain-m7.cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo -DCMAKE_INSTALL_PREFIX=~/src/gk -DCMAKE_FIND_ROOT_PATH=~/src/gk -DUNIX=ON -DSDL_LIBC=ON -DSDL_PTHREADS=ON -DSDL_THREADS=ON -DSDL_OPENGL=OFF -DSDL_OPENGLES=OFF -S SDL2-2.28.5/ -B build/sdl2
make -C build/sdl2 -j16 install

cmake -DCMAKE_TOOLCHAIN_FILE=../toolchain-m7.cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=~/src/gk -DCMAKE_FIND_ROOT_PATH=~/src/gk -DUNIX=ON jpeg-9f/ -B build/jpeg
make -C build/jpeg -j16 install

cmake -DCMAKE_TOOLCHAIN_FILE=../toolchain-m7.cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=~/src/gk -DCMAKE_FIND_ROOT_PATH=~/src/gk -DUNIX=ON SDL_image-release-2.8.2/ -B build/sdl2_image
make -C build/sdl2_image -j16 install

cmake -DCMAKE_TOOLCHAIN_FILE=../toolchain-m7.cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo -DCMAKE_INSTALL_PREFIX=~/src/gk -DCMAKE_FIND_ROOT_PATH=~/src/gk -DUNIX=ON -DSDL2MIXER_OPUS=OFF -DSDL2MIXER_MOD=OFF -DSDL2MIXER_MIDI=ON -DSDL2MIXER_MIDI_TIMIDITY=ON -DSDL2MIXER_MIDI_FLUIDSYNTH=OFF -DSDL2MIXER_WAVPACK=OFF -DSDL2MIXER_SAMPLES=OFF -DCMAKE_POSITION_INDEPENDENT_CODE=OFF SDL_mixer-release-2.8.0/ -B build/sdl2_mixer
make -C build/sdl2_mixer -j16 install

cmake -DCMAKE_TOOLCHAIN_FILE=../toolchain-m7.cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=~/src/gk -DCMAKE_FIND_ROOT_PATH=~/src/gk -DUNIX=ON freetype-2.13.2/ -B build/freetype2
make -C build/freetype2 -j16 install

cmake -DCMAKE_TOOLCHAIN_FILE=../toolchain-m7.cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=~/src/gk -DCMAKE_FIND_ROOT_PATH=~/src/gk -DUNIX=ON -DSDL2TTF_SAMPLES=OFF SDL_ttf-release-2.22.0/ -B build/sdl2_ttf
make -C build/sdl2_ttf -j16 install

cmake -DCMAKE_TOOLCHAIN_FILE=../toolchain-m7.cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo -DCMAKE_INSTALL_PREFIX=~/src/gk -DCMAKE_FIND_ROOT_PATH=~/src/gk -S SDL-1.2-main/ -B build/sdl12
make -C build/sdl12 -j16 install

cmake -DCMAKE_TOOLCHAIN_FILE=../toolchain-m7.cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=~/src/gk -DCMAKE_FIND_ROOT_PATH=~/src/gk -DBOOST_EXCLUDE_LIBRARIES=fiber\;wave\;asio\;log\;cobalt -DBOOST_RUNTIME_LINK=static -S boost-1.85.0/ -B build/boost
make -C build/boost -j16 install

cp -dpR glm/ ~/src/gk/include

cmake -DCMAKE_TOOLCHAIN_FILE=../toolchain-m7.cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo -DCMAKE_INSTALL_PREFIX=~/src/gk -DCMAKE_FIND_ROOT_PATH=~/src/gk -S mojoAL-main/ -B build/mojoAL
make -C build/mojoAL -j16 install
cp -dp ~/src/gk/lib/libmojoal.a ~/src/gk/lib/libal.a

cmake -DCMAKE_TOOLCHAIN_FILE=../toolchain-m7.cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo -DCMAKE_INSTALL_PREFIX=~/src/gk -DCMAKE_FIND_ROOT_PATH=~/src/gk -S libogg-1.3.5/ -B build/ogg
make -C build/ogg -j16 install

cmake -DCMAKE_TOOLCHAIN_FILE=../toolchain-m7.cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo -DCMAKE_INSTALL_PREFIX=~/src/gk -DCMAKE_FIND_ROOT_PATH=~/src/gk -S libvorbis-1.3.7/ -B build/vorbis
make -C build/vorbis -j16 install

cmake -DCMAKE_TOOLCHAIN_FILE=../toolchain-m7.cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo -DCMAKE_INSTALL_PREFIX=~/src/gk -DCMAKE_FIND_ROOT_PATH=~/src/gk -DPHYSFS_BUILD_TEST=OFF -S physfs-main/ -B build/physfs
make -C build/physfs -j16 install

cmake -DCMAKE_TOOLCHAIN_FILE=../toolchain-m7.cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo -DCMAKE_INSTALL_PREFIX=~/src/gk -DCMAKE_FIND_ROOT_PATH=~/src/gk -DHTTP_ONLY=ON -DCURL_ENABLE_SSL=OFF -DENABLE_IPV6=OFF -DBUILD_CURL_EXE=OFF -S curl-8.8.0/ -B build/curl
make -C build/curl -j16 install

cmake -DCMAKE_TOOLCHAIN_FILE=../toolchain-m7.cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo -DCMAKE_INSTALL_PREFIX=~/src/gk -DCMAKE_FIND_ROOT_PATH=~/src/gk -DDISABLE_DYNAMIC=ON -DSQ_DISABLE_INTERPRETER=ON -S squirrel3/ -B build/squirrel3
make -C build/squirrel3 -j16 install

mkdir -p build/tcl8
cd build/tcl8
CPPFLAGS="$GK_CPPFLAGS" CFLAGS="$GK_CFLAGS" CXXFLAGS="$GK_CXXFLAGS" LDFLAGS="$GK_LDFLAGS" ../../tcl8.6.14/unix/configure --host=arm-none-eabi --enable-static --disable-shared --disable-load --disable-unload --prefix=/home/jncronin/src/gk
make -j16 libtcl8.6.a
make -j16 libtclstub8.6.a
make install-headers
cp libtcl8.6.a /home/jncronin/src/gk/lib
cp libtclstub8.6.a /home/jncronin/src/gk/lib
cp tclConfig.sh /home/jncronin/src/gk/lib
cp ../../tcl8.6.14/unix/tclooConfig.sh /home/jncronin/src/gk/lib
mkdir -p /home/jncronin/src/gk/lib/pkgconfig
cp tcl.pc /home/jncronin/src/gk/lib/pkgconfig
cd ../..

cmake -DCMAKE_TOOLCHAIN_FILE=../toolchain-m7.cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo -DCMAKE_INSTALL_PREFIX=~/src/gk -DCMAKE_FIND_ROOT_PATH=~/src/gk -S lvgl-9.1.0/ -B build/lvgl9
make -C build/lvgl9 -j 16 install

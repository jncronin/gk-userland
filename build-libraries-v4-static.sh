#!/bin/bash

set -e

if [ -z "${SYSROOT}" ]; then
    SYSROOT=$HOME/opt/gkv4
fi

if [ -z "${TOOLSDIR}" ]; then
    TOOLSDIR=$HOME/opt/gkv4-build
fi

echo "Using GK toolchain in $TOOLSDIR.  Sysroot set to $SYSROOT."

# check for presence of g++
if [ ! -x $TOOLSDIR/bin/aarch64-none-gkos-c++ ]; then
    echo "Toolchain not found.  Please run ./build-toolchain.sh first."
    exit -1
fi

PATH="$TOOLSDIR/bin:$PATH"

export SYSROOT
export TOOLSDIR
export PATH

# cmake options
CMAKE_OPTS="-DCMAKE_POLICY_VERSION_MINIMUM=3.5  -DCMAKE_TOOLCHAIN_FILE=../toolchain-gkosv4-static.cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo -DCMAKE_INSTALL_PREFIX=$SYSROOT/usr"

#rm $SYSROOT/aarch64-none-gkos/include/iconv.h
cmake $CMAKE_OPTS -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -S libiconv-cmake-master/ -B build-v4/libiconv
make -C build-v4/libiconv -j16 install
cp -dpf $SYSROOT/usr/lib/liblibiconv.a $SYSROOT/usr/lib/libiconv.a
cp -dpf $SYSROOT/usr/lib/liblibcharset.a $SYSROOT/usr/lib/libcharset.a

cmake $CMAKE_OPTS -S zlib-1.3 -B build-v4/zlib
make -C build-v4/zlib -j16 install

cmake $CMAKE_OPTS -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -S libpng-1.6.40 -B build-v4/libpng
make -C build-v4/libpng -j16 install

mkdir -p build/mesa4
cd build/mesa4
../../Mesa-4.0.3/configure --host=aarch64-none-gkos --enable-static --disable-shared --prefix=$SYSROOT/usr
cd src && make -j16 install && cd ..
cd include && make install && cd ..
cd src-glu && make install && cd ..
cd ../..

# combine -lGL and -lOSMesa
echo -n -e "create $SYSROOT/usr/lib/libGL2.a\naddlib $SYSROOT/usr/lib/libGL.a\naddlib $SYSROOT/usr/lib/libOSMesa.a\nsave\nend\n" | aarch64-none-gkos-ar -M
mv $SYSROOT/usr/lib/libGL2.a $SYSROOT/usr/lib/libGL.a

cmake $CMAKE_OPTS -DUNIX=ON -DSDL_LIBC=ON -DSDL_PTHREADS=ON -DSDL_THREADS=ON -DSDL_OPENGL=OFF -DSDL_OPENGLES=OFF -DSDL_CLOCK_GETTIME=ON -S SDL2-2.28.5/ -B build-v4/sdl2
make -C build-v4/sdl2 -j16 install

cmake $CMAKE_OPTS -DUNIX=ON jpeg-9f/ -B build-v4/jpeg
make -C build-v4/jpeg -j16 install

cmake $CMAKE_OPTS -DUNIX=ON -DSDL2IMAGE_SAMPLES=OFF SDL_image-release-2.8.2/ -B build-v4/sdl2_image
make -C build-v4/sdl2_image -j16 install

cmake $CMAKE_OPTS -DUNIX=ON freetype-2.13.2/ -B build-v4/freetype2
make -C build-v4/freetype2 -j16 install

cmake $CMAKE_OPTS -DUNIX=ON -DSDL2TTF_SAMPLES=OFF SDL_ttf-release-2.22.0/ -B build-v4/sdl2_ttf
make -C build-v4/sdl2_ttf -j16 install

cmake $CMAKE_OPTS -S SDL-1.2-main/ -B build-v4/sdl12
make -C build-v4/sdl12 -j16 install
# copy SDL2s sdl-config/pkg-config scripts here
sed 's/SDL2/SDL/g' $SYSROOT/usr/bin/sdl2-config > $SYSROOT/usr/bin/sdl-config
chmod ugo+x $SYSROOT/usr/bin/sdl-config
sed 's/SDL2/SDL/g' $SYSROOT/usr/lib/pkgconfig/sdl2.pc | sed 's/sdl2/sdl/g' > $SYSROOT/usr/lib/pkgconfig/sdl.pc
chmod ugo+x $SYSROOT/usr/lib/pkgconfig/sdl.pc

mkdir -p build-v4/sdl_gfx
cd build-v4/sdl_gfx
../../SDL_gfx-2.0.27/configure --host=aarch64-none-gkos --disable-shared --enable-static --prefix=$SYSROOT/usr --disable-mmx --with-sdl-prefix=$SYSROOT/usr
make -j16 install
cd ../..

cmake $CMAKE_OPTS -DBOOST_EXCLUDE_LIBRARIES=fiber\;wave\;asio\;log\;cobalt\;test -DBOOST_RUNTIME_LINK=static -DBUILD_TESTING=OFF -S boost-1.85.0/ -B build-v4/boost
make -C build-v4/boost -j16 install

cp -dpR glm/ $SYSROOT/usr/include

cmake $CMAKE_OPTS  -S mojoAL-main/ -B build-v4/mojoAL
make -C build-v4/mojoAL -j16 install
cp -dp $SYSROOT/usr/lib/libmojoal.so $SYSROOT/usr/lib/libal.so
cp -dp $SYSROOT/usr/lib/libmojoal.so $SYSROOT/usr/lib/libopenal.so


cmake $CMAKE_OPTS -S libogg-1.3.5/ -B build-v4/ogg
make -C build-v4/ogg -j16 install

cmake $CMAKE_OPTS -S libvorbis-1.3.7/ -B build-v4/vorbis
make -C build-v4/vorbis -j16 install

cmake $CMAKE_OPTS -DPHYSFS_BUILD_TEST=OFF -S physfs-main/ -B build-v4/physfs
make -C build-v4/physfs -j16 install

cmake $CMAKE_OPTS -DHTTP_ONLY=ON -DCURL_ENABLE_SSL=OFF -DENABLE_IPV6=OFF -DBUILD_CURL_EXE=OFF -S curl-8.8.0/ -B build-v4/curl
make -C build-v4/curl -j16 install

cmake $CMAKE_OPTS -DDISABLE_DYNAMIC=ON -DSQ_DISABLE_INTERPRETER=ON -S squirrel3/ -B build-v4/squirrel3
make -C build-v4/squirrel3 -j16 install

mkdir -p build-v4/tcl8
cd build-v4/tcl8
../../tcl8.6.14/unix/configure --host=aarch64-none-gkos --disable-shared --enable-static --disable-load --disable-unload --prefix=$SYSROOT/usr
make -j16 libtcl8.6.a
make -j16 libtclstub8.6.a
make install-headers
cp libtcl8.6.a $SYSROOT/usr/lib
cp libtclstub8.6.a $SYSROOT/usr/lib
cp tclConfig.sh $SYSROOT/usr/lib
cp ../../tcl8.6.14/unix/tclooConfig.sh $SYSROOT/usr/lib
mkdir -p $SYSROOT/usr/lib/pkgconfig
cp tcl.pc $SYSROOT/usr/lib/pkgconfig
cd ../..

cmake $CMAKE_OPTS -S lvgl-9.1.0/ -B build-v4/lvgl9
make -C build-v4/lvgl9 -j 16 install

cp toolchain-gkosv4.cmake mpg123-1.32.7/ports
cp toolchain-gkosv4-static.cmake mpg123-1.32.7/ports
cmake $CMAKE_OPTS -S mpg123-1.32.7/ports/cmake -B build-v4/mpg123
make -C build-v4/mpg123 install

cmake $CMAKE_OPTS -DUNIX=ON -DSDL2MIXER_OPUS=OFF -DSDL2MIXER_MOD=OFF -DSDL2MIXER_MIDI=ON -DSDL2MIXER_MIDI_TIMIDITY=ON -DSDL2MIXER_MIDI_FLUIDSYNTH=OFF -DSDL2MIXER_WAVPACK=OFF -DSDL2MIXER_MP3_MINIMP3=OFF -DSDL2MIXER_MP3_MPG123=ON -DSDL2MIXER_MP3_MPG123_SHARED=OFF -DSDL2MIXER_SAMPLES=OFF SDL_mixer-release-2.8.0/ -B build-v4/sdl2_mixer
make -C build-v4/sdl2_mixer -j16 install

# build script doesn't like installing with static libraries
cmake $CMAKE_OPTS -S openal-soft-1.24.2 -B build-v4/openal -DALSOFT_BACKEND_SDL2=ON -DALSOFT_UTILS=OFF -DALSOFT_EXAMPLES=OFF -DALSOFT_INSTALL=OFF -DLIBTYPE=STATIC
make -C build-v4/openal -j 16 install
cp build/openal/openal.pc $SYSROOT/usr/lib/pkgconfig
cp build/openal/libopenal.a $SYSROOT/usr/lib

cd gettext-tiny-0.3.2
make CROSS_COMPILE=aarch64-none-gkos- CFLAGS="-g -O2" CC=aarch64-none-gkos-gcc AR=aarch64-none-gkos-ar prefix=$SYSROOT/usr install
cd ..

mkdir -p build-v4/glib
cd build-v4/glib
../../glib-2.20.5/configure --host=aarch64-none-gkos --enable-static --disable-shared --prefix=$SYSROOT/usr
make -j 16 install
cd ../..

cmake $CMAKE_OPTS -S fluidsynth-2.4.2 -B build-v4/fluidsynth -Denable-network=OFF
make -C build-v4/fluidsynth -j16 install

cmake $CMAKE_OPTS iir1-1.9.3/ -B build-v4/iir1
make -C build-v4/iir1/ -j16 install

cmake $CMAKE_OPTS -S munt-libmt32emu_2_7_1/ -B build-v4/libmt32emu -Dmunt_WITH_MT32EMU_SMF2WAV=OFF -Dmunt_WITH_MT32EMU_QT=OFF
make -C build-v4/libmt32emu/ -j16 install

echo "Successfully built gk libraries in $SYSROOT"

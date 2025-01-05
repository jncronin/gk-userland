#!/bin/bash

set -e

if [ -z "${SYSROOT}" ]; then
    SYSROOT=$HOME/opt/gk
fi

if [ -z "${TOOLSDIR}" ]; then
    TOOLSDIR=$HOME/opt/gk-build
fi

echo "Using GK toolchain in $TOOLSDIR.  Sysroot set to $SYSROOT."

# check for presence of g++
if [ ! -x $TOOLSDIR/bin/arm-none-gkos-c++ ]; then
    echo "Toolchain not found.  Please run ./build-toolchain.sh first."
    exit -1
fi

PATH="$TOOLSDIR/bin:$PATH"

export SYSROOT
export TOOLSDIR
export PATH

# cmake options
CMAKE_OPTS="-DCMAKE_TOOLCHAIN_FILE=../toolchain-gkos.cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo -DCMAKE_INSTALL_PREFIX=$SYSROOT/usr"

cmake $CMAKE_OPTS -S libiconv-cmake-master/ -B build/libiconv
make -C build/libiconv -j16 install
cp -dpf $SYSROOT/usr/lib/liblibiconv.a $SYSROOT/usr/lib/libiconv.a
cp -dpf $SYSROOT/usr/lib/liblibcharset.a $SYSROOT/usr/lib/libcharset.a

cmake $CMAKE_OPTS -S zlib-1.3 -B build/zlib
make -C build/zlib -j16 install
cp $SYSROOT/usr/lib/libzlib.a $SYSROOT/usr/lib/libz.a

cmake $CMAKE_OPTS -S libpng-1.6.40 -B build/libpng
make -C build/libpng -j16 install
cp $SYSROOT/usr/lib/liblibpng16.a $SYSROOT/usr/lib/libpng16.a

cmake $CMAKE_OPTS -S rain-1-s-shell/ -B build/s
make -C build/s -j16 install

cp -dpR nemagfx/inc/* $SYSROOT/usr/include
cp -dpR nemagfx/lib/* $SYSROOT/usr/lib

mkdir -p build/mesa4
cd build/mesa4
../../Mesa-4.0.3/configure --host=arm-none-gkos --enable-static --disable-shared --prefix=$SYSROOT/usr
cd src && make -j16 install && cd ..
cd include && make install && cd ..
cd src-glu && make install && cd ..
cd ../..

# combine -lGL and -lOSMesa
echo -n -e "create $SYSROOT/usr/lib/libGL2.a\naddlib $SYSROOT/usr/lib/libGL.a\naddlib $SYSROOT/usr/lib/libOSMesa.a\nsave\nend\n" | arm-none-gkos-ar -M
mv $SYSROOT/usr/lib/libGL2.a $SYSROOT/usr/lib/libGL.a

cmake $CMAKE_OPTS -DUNIX=ON -DSDL_LIBC=ON -DSDL_PTHREADS=ON -DSDL_THREADS=ON -DSDL_OPENGL=OFF -DSDL_OPENGLES=OFF -DSDL_CLOCK_GETTIME=ON -S SDL2-2.28.5/ -B build/sdl2
make -C build/sdl2 -j16 install

cmake $CMAKE_OPTS -DUNIX=ON jpeg-9f/ -B build/jpeg
make -C build/jpeg -j16 install

cmake $CMAKE_OPTS -DUNIX=ON -DCMAKE_POSITION_INDEPENDENT_CODE=OFF SDL_image-release-2.8.2/ -B build/sdl2_image
make -C build/sdl2_image -j16 install

cmake $CMAKE_OPTS -DUNIX=ON -DSDL2MIXER_OPUS=OFF -DSDL2MIXER_MOD=OFF -DSDL2MIXER_MIDI=ON -DSDL2MIXER_MIDI_TIMIDITY=ON -DSDL2MIXER_MIDI_FLUIDSYNTH=OFF -DSDL2MIXER_WAVPACK=OFF -DSDL2MIXER_MP3_MINIMP3=OFF -DSDL2MIXER_MP3_MPG123=ON -DSDL2MIXER_MP3_MPG123_SHARED=OFF -DSDL2MIXER_SAMPLES=OFF -DCMAKE_POSITION_INDEPENDENT_CODE=OFF SDL_mixer-release-2.8.0/ -B build/sdl2_mixer
make -C build/sdl2_mixer -j16 install

cmake $CMAKE_OPTS -DUNIX=ON freetype-2.13.2/ -B build/freetype2
make -C build/freetype2 -j16 install

cmake $CMAKE_OPTS -DUNIX=ON -DSDL2TTF_SAMPLES=OFF SDL_ttf-release-2.22.0/ -B build/sdl2_ttf
make -C build/sdl2_ttf -j16 install

cmake $CMAKE_OPTS -S SDL-1.2-main/ -B build/sdl12
make -C build/sdl12 -j16 install
# copy SDL2s sdl-config/pkg-config scripts here
sed 's/SDL2/SDL/g' $SYSROOT/usr/bin/sdl2-config > $SYSROOT/usr/bin/sdl-config
chmod ugo+x $SYSROOT/usr/bin/sdl-config
sed 's/SDL2/SDL/g' $SYSROOT/usr/lib/pkgconfig/sdl2.pc | sed 's/sdl2/sdl/g' > $SYSROOT/usr/lib/pkgconfig/sdl.pc
chmod ugo+x $SYSROOT/usr/lib/pkgconfig/sdl.pc

mkdir -p build/sdl_gfx
cd build/sdl_gfx
../../SDL_gfx-2.0.27/configure --host=arm-none-gkos --enable-static --disable-shared --prefix=$SYSROOT/usr --disable-mmx --with-sdl-prefix=$SYSROOT/usr
make -j16 install
cd ../..

cmake $CMAKE_OPTS -DBOOST_EXCLUDE_LIBRARIES=fiber\;wave\;asio\;log\;cobalt -DBOOST_RUNTIME_LINK=static -S boost-1.85.0/ -B build/boost
make -C build/boost -j16 install

cp -dpR glm/ ~/src/gk/include

cmake $CMAKE_OPTS  -S mojoAL-main/ -B build/mojoAL
make -C build/mojoAL -j16 install
cp -dp ~/src/gk/lib/libmojoal.a ~/src/gk/lib/libal.a

cmake $CMAKE_OPTS -S libogg-1.3.5/ -B build/ogg
make -C build/ogg -j16 install

cmake $CMAKE_OPTS -S libvorbis-1.3.7/ -B build/vorbis
make -C build/vorbis -j16 install

cmake $CMAKE_OPTS -DPHYSFS_BUILD_TEST=OFF -S physfs-main/ -B build/physfs
make -C build/physfs -j16 install

cmake $CMAKE_OPTS -DHTTP_ONLY=ON -DCURL_ENABLE_SSL=OFF -DENABLE_IPV6=OFF -DBUILD_CURL_EXE=OFF -S curl-8.8.0/ -B build/curl
make -C build/curl -j16 install

cmake $CMAKE_OPTS -DDISABLE_DYNAMIC=ON -DSQ_DISABLE_INTERPRETER=ON -S squirrel3/ -B build/squirrel3
make -C build/squirrel3 -j16 install

mkdir -p build/tcl8
cd build/tcl8
../../tcl8.6.14/unix/configure --host=arm-none-gkos --enable-static --disable-shared --disable-load --disable-unload --prefix=$SYSROOT/usr
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

cmake $CMAKE_OPTS -S lvgl-9.1.0/ -B build/lvgl9
make -C build/lvgl9 -j 16 install

mkdir -p build/mpg123
cd build/mpg123
../../mpg123-1.32.7/configure --host=arm-none-gkos --enable-static --disable-shared --disable-components --enable-libmpg123 --with-audio=dummy --prefix=$SYSROOT/usr
make -j16 install
cd ../..

echo "Successfully built gk libraries in $SYSROOT"

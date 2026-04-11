#!/bin/sh

# first build libdrm - needs meson - this from the gk-userland root
meson setup --cross-file meson_cross_file.txt -Dintel=disabled -Dradeon=disabled -Damdgpu=disabled -Dnouveau=disabled -Dvmwgfx=disabled -Domap=disabled -Dexynos=disabled -Dfreedreno=disabled -Dtegra=disabled -Dvc4=disabled -Detnaviv=enabled -Dtests=false -Dvalgrind=disabled -Dman-pages=disabled --prefix ~/opt/gkv4/usr build-v4/libdrm/ libdrm-2.4.131/

meson compile -C build-v4/libdrm
meson install -C build-v4/libdrm

# messa build
meson setup -Dgallium-drivers=etnaviv -Dvulkan-drivers= -Dplatforms= -Degl=enabled -Dglx=disabled -Dglx-direct=false -Ddefault_library=static --cross-file cross_file.txt -Dbuildtype=debug -Db_ndebug=false -Dcmake_prefix_path=~/opt/gkv4/usr/lib/cmake -Dshader-cache=enabled -Dshader-cache-default=true -Ddri-drivers-path=lib --prefix ~/opt/gkv4/usr builddir-etnaviv

# add -Dbuildtype=release -Db_ndebug=true for release builds

meson compile -C builddir-etnaviv
meson install -C builddir-etnaviv

mv ~/opt/gkv4/usr/lib/libgallium-26.0.4.so ~/opt/gkv4/usr/lib/libgallium.a
cp ~/opt/gkv4/usr/lib/gbm/dri_gbm.a ~/opt/gkv4/usr/lib/libdri_gbm.a

# combine the various mesa libraries into libGL.a
echo -n -e "create $SYSROOT/usr/lib/libGL2.a\naddlib $SYSROOT/usr/lib/libglapi_bridge.a\naddlib $SYSROOT/usr/lib/libglapi.a\naddlib $SYSROOT/usr/lib/libgallium.a\naddlib $SYSROOT/usr/lib/libexpat.a\naddlib $SYSROOT/usr/lib/libgbm.a\naddlib $SYSROOT/usr/lib/libdri_gbm.a\naddlib $SYSROOT/usr/lib/libdrm.a\naddlib $SYSROOT/usr/lib/libllvm.a\nsave\nend\n" | aarch64-none-gkos-ar -M
mv $SYSROOT/usr/lib/libGL2.a $SYSROOT/usr/lib/libGL.a

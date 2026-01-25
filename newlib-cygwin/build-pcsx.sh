#!/bin/sh

CROSS_COMPILE=aarch64-none-gkos- CFLAGS="-g -DALLOW_LIGHTREC_ON_ARM" ./configure --disable-dynamic --dynarec=lightrec
make -j16
./make_gkv4_package.sh

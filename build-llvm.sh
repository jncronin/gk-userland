#!/bin/sh

ADDLIBS=`find $SYSROOT/usr/lib/libLLVM* | awk '{print "addlib " $1}' -`
ARSCRIPT="create $SYSROOT/usr/lib/libllvm.a\n$ADDLIBS\nsave\nend\n"
echo -n "$ARSCRIPT" | aarch64-none-gkos-ar -M

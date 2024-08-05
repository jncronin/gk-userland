cmake_minimum_required(VERSION 3.6 FATAL_ERROR)

set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)

set(GAMEKID ON)
set(UNIX ON)
set(BUILD_SHARED_LIBS OFF)

set(CMAKE_C_COMPILER arm-none-gkos-gcc)
set(CMAKE_CXX_COMPILER arm-none-gkos-g++)

set(CMAKE_SYSROOT "$ENV{SYSROOT}")
set(CMAKE_FIND_ROOT_PATH "$ENV{SYSROOT}" "$ENV{SYSROOT}/usr")

set(ENV{PKG_CONFIG_PATH} "")
set(ENV{PKG_CONFIG_LIBDIR} "${CMAKE_SYSROOT}/usr/lib/pkgconfig:${CMAKE_SYSROOT}/usr/share/pkgconfig")
#set(ENV{PKG_CONFIG_SYSROOT_DIR} ${CMAKE_SYSROOT})

# adjust the default behavior of the FIND_XXX() commands:
# search programs in the host environment
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)

# search headers and libraries in the target environment
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

set(CPACK_GENERATOR "TGZ")
set(CPACK_SYSTEM_NAME "gk")

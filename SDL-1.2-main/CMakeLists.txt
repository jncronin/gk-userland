cmake_minimum_required(VERSION 3.0)

project(SDL12)

add_library(sdl)

file(GLOB SOURCES CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/audio/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/cdrom/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/cpuinfo/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/events/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/file/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/joystick/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/stdlib/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/thread/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/timer/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/video/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/audio/disk/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/video/dummy/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/joystick/dummy/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/cdrom/dummy/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/thread/generic/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/timer/dummy/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/loadso/dummy/*.c
)

target_sources(sdl PRIVATE
    ${SOURCES})

target_include_directories(sdl PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include)

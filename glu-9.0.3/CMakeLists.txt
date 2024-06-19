cmake_minimum_required(VERSION 3.11 FATAL_ERROR)

project(glu C CXX)

add_library(glu)

file(GLOB_RECURSE GLU_SOURCES CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*/*.cc
)

target_sources(glu PRIVATE ${GLU_SOURCES})

target_include_directories(glu
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src/include
        ${CMAKE_CURRENT_SOURCE_DIR}/src/libnurbs/internals
        ${CMAKE_CURRENT_SOURCE_DIR}/src/libnurbs/interface
        ${CMAKE_CURRENT_SOURCE_DIR}/src/libnurbs/nurbtess
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_definitions(glu
    PRIVATE
        LIBRARYBUILD
)

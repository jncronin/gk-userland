cmake_minimum_required(VERSION 3.11 FATAL_ERROR)

project(gkgl C CXX ASM)

# gkgl_base is combined with libEGL to create libGKGL
add_library(gkgl_base)

target_sources(gkgl_base
PRIVATE
	src-gkgl/gkgl.cpp
	src-gkgl/gkgl_sdl2.cpp
)

target_include_directories(gkgl_base
PRIVATE
	"${CMAKE_CURRENT_SOURCE_DIR}/../gloss-gk/inc-pthread"
	"${CMAKE_CURRENT_SOURCE_DIR}/../gloss-gk/gk-userlandinterface"
	"${CMAKE_CURRENT_SOURCE_DIR}/../SDL2-2.28.5/src/render"
	"${CMAKE_CURRENT_SOURCE_DIR}/../SDL2-2.28.5/src/render/opengl"
)

install(TARGETS gkgl_base)

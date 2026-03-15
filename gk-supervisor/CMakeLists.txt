cmake_minimum_required(VERSION 3.11 FATAL_ERROR)

project(gksupervisor C CXX)

add_executable(gksupervisor)

find_package(PkgConfig REQUIRED)
find_package(SDL2 REQUIRED)
find_package(SDL2_mixer REQUIRED)
find_package(squirrel REQUIRED)
find_package(Freetype REQUIRED)

pkg_check_modules(LVGL REQUIRED lvgl)
pkg_check_modules(PNG REQUIRED libpng)

set_property(TARGET gksupervisor PROPERTY CXX_STANDARD 20)

file(GLOB_RECURSE GKSUPERVISOR_SOURCES CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
)

target_sources(gksupervisor
    PRIVATE
        ${GKSUPERVISOR_SOURCES}
        ${CMAKE_CURRENT_SOURCE_DIR}/../gk-joycalib/src/joystick_conf.cpp
)

target_compile_definitions(gksupervisor
    PRIVATE
        LV_COLOR_DEPTH=32
)

target_include_directories(gksupervisor
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/inc
        ${LVGL_INCLUDE_DIRS}
        ${PNG_INCLUDE_DIRS}
        ${CMAKE_CURRENT_SOURCE_DIR}/../gk-joycalib/inc
)

target_link_libraries(gksupervisor
    PRIVATE
        SDL2::SDL2-static
        SDL2_mixer::SDL2_mixer-static
        SDL2::SDL2-static
        squirrel::squirrel_static
        squirrel::sqstdlib_static
        ${LVGL_LIBRARIES}
        freetype
        ${PNG_LIBRARIES}
        z
)

install(TARGETS gksupervisor)
include(CPack)

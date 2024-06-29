cmake_minimum_required(VERSION 3.11 FATAL_ERROR)

project(gkmenu C CXX)

add_executable(gkmenu)

find_package(PkgConfig REQUIRED)
find_package(SDL2 REQUIRED)
find_package(squirrel REQUIRED)
find_package(Freetype REQUIRED)

pkg_check_modules(LVGL REQUIRED lvgl)
pkg_check_modules(PNG REQUIRED libpng)

file(GLOB_RECURSE GKMENU_SOURCES CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
)

target_sources(gkmenu
    PRIVATE
        ${GKMENU_SOURCES}
)

target_include_directories(gkmenu
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/inc
        ${LVGL_INCLUDE_DIRS}
        ${PNG_INCLUDE_DIRS}
)

target_link_libraries(gkmenu
    PRIVATE
#        SDL2::SDL2
        squirrel::squirrel_static
        ${LVGL_LIBRARIES}
        freetype
        ${PNG_LIBRARIES}
        z
)

install(TARGETS gkmenu)
include(CPack)

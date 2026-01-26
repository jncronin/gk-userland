cmake_minimum_required(VERSION 3.11 FATAL_ERROR)

project(gkjoy C CXX)

add_executable(gkjoy)

find_package(SDL2 REQUIRED)

set_property(TARGET gkjoy PROPERTY CXX_STANDARD 20)

file(GLOB_RECURSE GKJOY_SOURCES CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
)

target_sources(gkjoy
    PRIVATE
        ${GKJOY_SOURCES}
)

target_link_libraries(gkjoy
    PRIVATE
        SDL2::SDL2-static
)

install(TARGETS gkjoy)
include(CPack)

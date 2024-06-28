cmake_minimum_required(VERSION 3.11 FATAL_ERROR)

project(gkmenu C CXX)

add_executable(gkmenu)

find_package(PkgConfig REQUIRED)
find_package(SDL2 REQUIRED)

pkg_check_modules(SQUIRREL REQUIRED squirrel3)

file(GLOB_RECURSE GKMENU_SOURCES
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
        ${SQUIRREL_INCLUDE_DIRS}
)

target_link_libraries(gkmenu
    PRIVATE
        SDL2::SDL2
        ${SQUIRREL_LIBRARIES}
)

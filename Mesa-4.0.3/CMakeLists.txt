cmake_minimum_required(VERSION 3.11 FATAL_ERROR)

project(Mesa C)

file(GLOB SOURCES_MESA CONFIGURE_DEPENDS
	${CMAKE_CURRENT_SOURCE_DIR}/src/*.c
)

file(GLOB SOURCES_ARRAY_CACHE CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/src/array_cache/*.c
)

file(GLOB SOURCES_MATH CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/src/math/*.c
)

file(GLOB SOURCES_OSMESA CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/src/OSMesa/*.c
)

file(GLOB SOURCES_SWRAST CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/src/swrast/*.c
)

file(GLOB SOURCES_SWRAST_SETUP CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/src/swrast_setup/*.c
)

file(GLOB SOURCES_GLU CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/src-glu/*.c
)

add_library(GL)

target_sources(GL
    PRIVATE
        ${SOURCES_MESA}
        ${SOURCES_ARRAY_CACHE}
        ${SOURCES_MATH}
        ${SOURCES_OSMESA}
        ${SOURCES_SWRAST}
        ${SOURCES_SWRAST_SETUP}
)

target_include_directories(GL
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_include_directories(GL
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/src/array_cache
        ${CMAKE_CURRENT_SOURCE_DIR}/src/math
        ${CMAKE_CURRENT_SOURCE_DIR}/src/OSMesa
        ${CMAKE_CURRENT_SOURCE_DIR}/src/swrast
        ${CMAKE_CURRENT_SOURCE_DIR}/src/swrast_setup
)

add_library(GLU)

target_sources(GLU
    PRIVATE
        ${SOURCES_GLU}
)

target_include_directories(GLU
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_include_directories(GLU
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src-glu
)

install(TARGETS GL GLU)

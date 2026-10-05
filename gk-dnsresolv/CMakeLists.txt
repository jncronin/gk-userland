cmake_minimum_required(VERSION 3.11 FATAL_ERROR)

project(dnsresolv C CXX)

add_executable(dnsresolv)

set_property(TARGET dnsresolv PROPERTY CXX_STANDARD 20)

file(GLOB_RECURSE dnsresolv_SOURCES CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
)

target_sources(dnsresolv
    PRIVATE
        ${dnsresolv_SOURCES}
)

target_include_directories(dnsresolv
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/inc
)

# The following for now.  gloss-gk (particularly resolv.cpp) needs libstdc++ functions therefore
#  ensure they get linked in the appropriate order.
target_link_libraries(dnsresolv
    PRIVATE
        gloss-gk
        stdc++
)

install(TARGETS dnsresolv)
include(CPack)

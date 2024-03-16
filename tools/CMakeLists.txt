cmake_minimum_required(VERSION 3.11 FATAL_ERROR)

project(tools C CXX)

add_library(common)

target_sources(common
    PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/common/args.cpp"
)
target_include_directories(common
    PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/common"
)

add_executable(ls)

target_sources(ls
    PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/ls/ls.cpp"
)
target_link_libraries(ls
    PRIVATE
        common
)

add_executable(tftpd)

target_sources(tftpd
    PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/tftpd/tftpd.cpp"
)
target_link_libraries(tftpd
    PRIVATE
        common
)

install(TARGETS ls tftpd)

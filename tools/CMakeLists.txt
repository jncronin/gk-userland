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

add_executable(echo)

target_sources(echo
    PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/echo/echo.cpp"
)
target_link_libraries(echo
    PRIVATE
        common
)

add_executable(ed)

target_sources(ed
    PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/ed/ed.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/ed/buffer.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/ed/file.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/ed/parser.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/ed/print.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/ed/edit.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/ed/misc.cpp"
)
target_link_libraries(ed
    PRIVATE
        common
)

add_executable(sh)

get_target_property(ED_SOURCES ed SOURCES)
target_sources(sh
    PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/shell/shell.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/shell/tokenize.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/ls/ls.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/cat/cat.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/cd/cd.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/mkdir/mkdir.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/rm/rm.cpp"
        ${ED_SOURCES}
)
target_link_libraries(sh
    PRIVATE
        common
)
target_compile_definitions(sh
    PRIVATE
        IN_SHELL
)

add_executable(telnetd)
get_target_property(SH_SOURCES sh SOURCES)
target_sources(telnetd
    PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/telnetd/telnetd.cpp"
        ${SH_SOURCES}
)
target_link_libraries(telnetd
    PRIVATE
        common
)
target_compile_definitions(telnetd
    PRIVATE
        IN_TELNETD
)

add_executable(console)
target_sources(console
    PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/console/console.cpp"
        ${SH_SOURCES}
)
target_link_libraries(console
    PRIVATE
        common
)
target_compile_definitions(console
    PRIVATE
        IN_CONSOLE
)


install(TARGETS ls tftpd echo ed telnetd sh console)

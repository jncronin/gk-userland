cmake_minimum_required(VERSION 3.6 FATAL_ERROR)

set(TARGET_CPU "cortex-m7")

set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)

set(GAMEKID ON)
set(UNIX ON)
set(BUILD_SHARED_LIBS OFF)

set(CMAKE_C_COMPILER arm-none-eabi-gcc)
set(CMAKE_CXX_COMPILER arm-none-eabi-g++)

set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
set(CMAKE_C_STANDARD_LIBRARIES "-lm")
set(CMAKE_CXX_STANDARD_LIBRARIES "-lm")

set(COMMON_FLAGS "-mthumb -mcpu=${TARGET_CPU} -mfloat-abi=hard -mfpu=fpv5-d16 -ffast-math")
set(C_CXX_FLAGS  "-include sys/gk.h -ffunction-sections -fdata-sections -ffreestanding -D__GAMEKID__ -D_POSIX_THREADS=1  -Dunix -DUNIX -D_POSIX_MONOTONIC_CLOCK -D_POSIX_TIMERS=1 -D_POSIX_READER_WRITER_LOCKS=1 -I$ENV{HOME}/src/gk/include")
set(CXX_FLAGS    "-fno-threadsafe-statics -Wno-psabi")

set(CMAKE_C_FLAGS_INIT          "${COMMON_FLAGS} ${C_CXX_FLAGS}"              CACHE STRING "" FORCE)
set(CMAKE_CXX_FLAGS_INIT        "${COMMON_FLAGS} ${C_CXX_FLAGS} ${CXX_FLAGS}" CACHE STRING "" FORCE)
set(CMAKE_ASM_FLAGS_INIT        "${COMMON_FLAGS} -x assembler-with-cpp"       CACHE STRING "" FORCE)
set(CMAKE_EXE_LINKER_FLAGS_INIT "-Wl,--entry,_mainCRTStartup -Wl,--section-start,.init=0 -Wl,-Ttext,0x32 -Wl,-z,max-page-size=32 -Wl,--gc-sections -L$ENV{HOME}/src/gk/lib -lm -Wl,--whole-archive -llibcharset $ENV{HOME}/src/gk/lib/libgloss-gk.a $ENV{HOME}/src/gk/lib/libgk.a -Wl,--no-whole-archive -Wl,-q"                           CACHE STRING "" FORCE)

set(CMAKE_C_FLAGS_DEBUG     "-Og -g"          CACHE STRING "" FORCE)
set(CMAKE_CXX_FLAGS_DEBUG   "-Og -g"          CACHE STRING "" FORCE)
set(CMAKE_C_FLAGS_RELEASE   "-O3 -DNDEBUG" CACHE STRING "" FORCE)
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG" CACHE STRING "" FORCE)

set(CMAKE_FIND_ROOT_PATH "$ENV{HOME}/src/gk")
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

set(CMAKE_INSTALL_PREFIX "$ENV{HOME}/src/gk")
set(CMAKE_STAGING_PREFIX "$ENV{HOME}/src/gk")
set(CMAKE_PREFIX_PATH "$ENV{HOME}/src/gk")

set(CMAKE_EXPORT_COMPILE_COMMANDS             ON)
set(CMAKE_C_USE_RESPONSE_FILE_FOR_INCLUDES    ON)
set(CMAKE_C_USE_RESPONSE_FILE_FOR_LIBRARIES   ON)
set(CMAKE_C_USE_RESPONSE_FILE_FOR_OBJECTS     ON)
set(CMAKE_CXX_USE_RESPONSE_FILE_FOR_INCLUDES  ON)
set(CMAKE_CXX_USE_RESPONSE_FILE_FOR_LIBRARIES ON)
set(CMAKE_CXX_USE_RESPONSE_FILE_FOR_OBJECTS   ON)
set(CMAKE_NINJA_FORCE_RESPONSE_FILE           ON)


set(CPACK_GENERATOR "TGZ")
set(CPACK_SYSTEM_NAME "gk")

function(gk_generate_package)
    string(TIMESTAMP TSTAMP %s)
    set(FNAME "${CMAKE_CURRENT_BINARY_DIR}/gk-${TSTAMP}.tar")
    message("generating ${FNAME}")

    foreach(tname IN LISTS ARGV)
        message("tname: ${tname}")
        if(TARGET ${tname})
            message("target")
            list(APPEND EXFNAMES "$<TARGET_FILE:${tname}>")
        else()
            message("not target")
            list(APPEND EXFNAMES "${tname}")
        endif()
    endforeach()

    add_custom_target(create_gk_package
        ALL
        COMMAND ${CMAKE_COMMAND} -E echo "generating ${FNAME} from $<JOIN:${EXFNAMES},,>"
        COMMAND ${CMAKE_COMMAND} -E tar "cvf" "${FNAME}" ${EXFNAMES}
        VERBATIM
    )

    add_custom_target(RerunCmake ${CMAKE_COMMAND} ${CMAKE_SOURCE_DIR})
    add_dependencies(create_gk_package RerunCmake)
endfunction()

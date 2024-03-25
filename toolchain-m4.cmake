cmake_minimum_required(VERSION 3.6 FATAL_ERROR)

set(TARGET_CPU "cortex-m4")

set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR ${TARGET_CPU})

set(GAMEKID ON)
set(UNIX ON)
set(BUILD_SHARED_LIBS OFF)

set(CMAKE_C_COMPILER arm-none-eabi-gcc)
set(CMAKE_CXX_COMPILER arm-none-eabi-g++)

set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
set(CMAKE_C_STANDARD_LIBRARIES "-lm")
set(CMAKE_CXX_STANDARD_LIBRARIES "-lm")

set(COMMON_FLAGS "-mthumb -mcpu=${TARGET_CPU} -mfloat-abi=hard -mfpu=fpv5-sp-d16 -ffast-math")
set(C_CXX_FLAGS  "--specs=nano.specs -ffunction-sections -fdata-sections -ffreestanding -D__GAMEKID__ -D_POSIX_THREADS -I$ENV{HOME}/src/gk/include")
set(CXX_FLAGS    "-fno-exceptions -fno-rtti -fno-threadsafe-statics -Wno-psabi")

set(CMAKE_C_FLAGS_INIT          "${COMMON_FLAGS} ${C_CXX_FLAGS}"              CACHE STRING "" FORCE)
set(CMAKE_CXX_FLAGS_INIT        "${COMMON_FLAGS} ${C_CXX_FLAGS} ${CXX_FLAGS}" CACHE STRING "" FORCE)
set(CMAKE_ASM_FLAGS_INIT        "${COMMON_FLAGS} -x assembler-with-cpp"       CACHE STRING "" FORCE)
set(CMAKE_EXE_LINKER_FLAGS_INIT "-Wl,--entry,_mainCRTStartup -Wl,--section-start,.init=0 -Wl,-Ttext,0x32 -Wl,-z,max-page-size=32 -Wl,--gc-sections -L$ENV{HOME}/src/gk/lib -lm -Wl,--whole-archive -llibcharset $ENV{HOME}/src/gk/lib/libgloss-gk.a $ENV{HOME}/src/gk/lib/libgk.a -Wl,--no-whole-archive -Wl,-q"                           CACHE STRING "" FORCE)

set(CMAKE_C_FLAGS_DEBUG     "-Og -g"          CACHE STRING "" FORCE)
set(CMAKE_CXX_FLAGS_DEBUG   "-Og -g"          CACHE STRING "" FORCE)
set(CMAKE_C_FLAGS_RELEASE   "-Os -g -DNDEBUG" CACHE STRING "" FORCE)
set(CMAKE_CXX_FLAGS_RELEASE "-Os -g -DNDEBUG" CACHE STRING "" FORCE)

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


cmake_minimum_required(VERSION 3.0)

project(SDL12)

add_library(SDL)

file(GLOB SOURCES CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/audio/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/cdrom/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/cpuinfo/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/events/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/file/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/joystick/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/stdlib/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/thread/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/timer/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/video/*.c
#    ${CMAKE_CURRENT_SOURCE_DIR}/src/audio/disk/*.c
#    ${CMAKE_CURRENT_SOURCE_DIR}/src/video/dummy/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/joystick/gk/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/cdrom/dummy/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/thread/pthread/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/timer/unix/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/loadso/dummy/*.c

    ${CMAKE_CURRENT_SOURCE_DIR}/src/video/gk/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/audio/gk/*.c
)

target_sources(SDL PRIVATE
    ${SOURCES})

target_include_directories(SDL PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_library(SDLmain)

target_sources(SDLmain PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main/dummy/SDL_dummy_main.c)

target_include_directories(SDLmain PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include)

#target_link_libraries(SDL
#    INTERFACE
#        OSMesa GL)

install(TARGETS SDL SDLmain)

set(HDRS 
    ${CMAKE_CURRENT_SOURCE_DIR}/include/SDL.h 
    ${CMAKE_CURRENT_SOURCE_DIR}/include/SDL_active.h 
    ${CMAKE_CURRENT_SOURCE_DIR}/include/SDL_audio.h 
    ${CMAKE_CURRENT_SOURCE_DIR}/include/SDL_byteorder.h 
    ${CMAKE_CURRENT_SOURCE_DIR}/include/SDL_cdrom.h 
    ${CMAKE_CURRENT_SOURCE_DIR}/include/SDL_cpuinfo.h 
    ${CMAKE_CURRENT_SOURCE_DIR}/include/SDL_endian.h 
    ${CMAKE_CURRENT_SOURCE_DIR}/include/SDL_error.h 
    ${CMAKE_CURRENT_SOURCE_DIR}/include/SDL_events.h 
    ${CMAKE_CURRENT_SOURCE_DIR}/include/SDL_getenv.h 
    ${CMAKE_CURRENT_SOURCE_DIR}/include/SDL_joystick.h 
    ${CMAKE_CURRENT_SOURCE_DIR}/include/SDL_keyboard.h 
    ${CMAKE_CURRENT_SOURCE_DIR}/include/SDL_keysym.h 
    ${CMAKE_CURRENT_SOURCE_DIR}/include/SDL_loadso.h 
    ${CMAKE_CURRENT_SOURCE_DIR}/include/SDL_main.h 
    ${CMAKE_CURRENT_SOURCE_DIR}/include/SDL_mouse.h 
    ${CMAKE_CURRENT_SOURCE_DIR}/include/SDL_mutex.h 
    ${CMAKE_CURRENT_SOURCE_DIR}/include/SDL_name.h 
    ${CMAKE_CURRENT_SOURCE_DIR}/include/SDL_opengl.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/SDL_platform.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/SDL_quit.h 
    ${CMAKE_CURRENT_SOURCE_DIR}/include/SDL_rwops.h 
    ${CMAKE_CURRENT_SOURCE_DIR}/include/SDL_stdinc.h 
    ${CMAKE_CURRENT_SOURCE_DIR}/include/SDL_syswm.h 
    ${CMAKE_CURRENT_SOURCE_DIR}/include/SDL_thread.h 
    ${CMAKE_CURRENT_SOURCE_DIR}/include/SDL_timer.h 
    ${CMAKE_CURRENT_SOURCE_DIR}/include/SDL_types.h 
    ${CMAKE_CURRENT_SOURCE_DIR}/include/SDL_version.h 
    ${CMAKE_CURRENT_SOURCE_DIR}/include/SDL_video.h 
    ${CMAKE_CURRENT_SOURCE_DIR}/include/begin_code.h 
    ${CMAKE_CURRENT_SOURCE_DIR}/include/close_code.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/SDL_config.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/SDL_config_gk.h
)

install(FILES ${HDRS} DESTINATION include/SDL)




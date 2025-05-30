cmake_minimum_required(VERSION 3.11 FATAL_ERROR)

project(jpeg9 C)

add_library(jpeg)

target_sources(jpeg
    PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/jaricom.c"
        "${CMAKE_CURRENT_SOURCE_DIR}/jcapimin.c"
        "${CMAKE_CURRENT_SOURCE_DIR}/jcapistd.c"
        "${CMAKE_CURRENT_SOURCE_DIR}/jcarith.c"
        "${CMAKE_CURRENT_SOURCE_DIR}/jccoefct.c"
        "${CMAKE_CURRENT_SOURCE_DIR}/jccolor.c"
        "${CMAKE_CURRENT_SOURCE_DIR}/jcdctmgr.c"
        "${CMAKE_CURRENT_SOURCE_DIR}/jchuff.c"
        "${CMAKE_CURRENT_SOURCE_DIR}/jcinit.c"
        "${CMAKE_CURRENT_SOURCE_DIR}/jcmainct.c"
        "${CMAKE_CURRENT_SOURCE_DIR}/jcmarker.c"
        "${CMAKE_CURRENT_SOURCE_DIR}/jcmaster.c"
        "${CMAKE_CURRENT_SOURCE_DIR}/jcomapi.c"
        "${CMAKE_CURRENT_SOURCE_DIR}/jcparam.c"
        "${CMAKE_CURRENT_SOURCE_DIR}/jcprepct.c"
        "${CMAKE_CURRENT_SOURCE_DIR}/jcsample.c"
        "${CMAKE_CURRENT_SOURCE_DIR}/jctrans.c"
        "${CMAKE_CURRENT_SOURCE_DIR}/jdapimin.c"
        "${CMAKE_CURRENT_SOURCE_DIR}/jdapistd.c"
        "${CMAKE_CURRENT_SOURCE_DIR}/jdarith.c"
        "${CMAKE_CURRENT_SOURCE_DIR}/jdatadst.c"
        "${CMAKE_CURRENT_SOURCE_DIR}/jdatasrc.c"
        "${CMAKE_CURRENT_SOURCE_DIR}/jdcoefct.c"
        "${CMAKE_CURRENT_SOURCE_DIR}/jdcolor.c"
        "${CMAKE_CURRENT_SOURCE_DIR}/jddctmgr.c"
        "${CMAKE_CURRENT_SOURCE_DIR}/jdhuff.c"
        "${CMAKE_CURRENT_SOURCE_DIR}/jdinput.c"
        "${CMAKE_CURRENT_SOURCE_DIR}/jdmainct.c"
        "${CMAKE_CURRENT_SOURCE_DIR}/jdmarker.c"
        "${CMAKE_CURRENT_SOURCE_DIR}/jdmaster.c"
        "${CMAKE_CURRENT_SOURCE_DIR}/jdmerge.c"
        "${CMAKE_CURRENT_SOURCE_DIR}/jdpostct.c"
        "${CMAKE_CURRENT_SOURCE_DIR}/jdsample.c"
        "${CMAKE_CURRENT_SOURCE_DIR}/jdtrans.c"
        "${CMAKE_CURRENT_SOURCE_DIR}/jerror.c"
        "${CMAKE_CURRENT_SOURCE_DIR}/jfdctflt.c"
        "${CMAKE_CURRENT_SOURCE_DIR}/jfdctfst.c"
        "${CMAKE_CURRENT_SOURCE_DIR}/jfdctint.c"
        "${CMAKE_CURRENT_SOURCE_DIR}/jidctflt.c"
        "${CMAKE_CURRENT_SOURCE_DIR}/jidctfst.c"
        "${CMAKE_CURRENT_SOURCE_DIR}/jidctint.c"
        "${CMAKE_CURRENT_SOURCE_DIR}/jquant1.c"
        "${CMAKE_CURRENT_SOURCE_DIR}/jquant2.c"
        "${CMAKE_CURRENT_SOURCE_DIR}/jutils.c"
        "${CMAKE_CURRENT_SOURCE_DIR}/jmemmgr.c"
        "${CMAKE_CURRENT_SOURCE_DIR}/jmemansi.c"
)

set_target_properties(jpeg PROPERTIES PUBLIC_HEADER "jerror.h;jmorecfg.h;jpeglib.h;jconfig.h;jpegint.h")

INSTALL(TARGETS jpeg
    LIBRARY DESTINATION lib
    PUBLIC_HEADER DESTINATION include)

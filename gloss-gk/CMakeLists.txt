cmake_minimum_required(VERSION 3.11 FATAL_ERROR)

project(gloss-gk C CXX ASM)

file(GLOB_RECURSE SOURCES_GLOSS CONFIGURE_DEPENDS
	${CMAKE_CURRENT_SOURCE_DIR}/src-gloss/*.c
	${CMAKE_CURRENT_SOURCE_DIR}/src-gloss/*.cpp
)

file(GLOB_RECURSE SOURCES_GK CONFIGURE_DEPENDS
	${CMAKE_CURRENT_SOURCE_DIR}/src-gk/*.c
)

file(GLOB_RECURSE SOURCES_PTHREAD CONFIGURE_DEPENDS
	${CMAKE_CURRENT_SOURCE_DIR}/src-pthread/*.c
)

add_library(gloss-gk)

target_sources(gloss-gk
	PRIVATE
	${SOURCES_GLOSS}
)

target_include_directories(gloss-gk
	PRIVATE
	"${CMAKE_CURRENT_SOURCE_DIR}/src-gloss"
	"${CMAKE_CURRENT_SOURCE_DIR}/gk-userlandinterface"
	"${CMAKE_CURRENT_SOURCE_DIR}/inc-gk"
	"${CMAKE_CURRENT_SOURCE_DIR}/cmsis_device_h7/Include"
	"${CMAKE_CURRENT_SOURCE_DIR}/cmsis_core/Include"
	)

add_library(gk)

target_sources(gk
	PRIVATE
	${SOURCES_GK}
)

target_include_directories(gk
	PRIVATE
	"${CMAKE_CURRENT_SOURCE_DIR}/gk-userlandinterface"
	"${CMAKE_CURRENT_SOURCE_DIR}/inc-gk"
	"${CMAKE_CURRENT_SOURCE_DIR}/cmsis_device_h7/Include"
	"${CMAKE_CURRENT_SOURCE_DIR}/cmsis_core/Include")

set_target_properties(gk PROPERTIES PUBLIC_HEADER "cmsis_device_h7/Include/stm32h7xx.h;cmsis_device_h7/Include/stm32h747xx.h;cmsis_core/Include/cmsis_compiler.h;cmsis_core/Include/cmsis_gcc.h;cmsis_core/Include/cmsis_version.h;cmsis_core/Include/core_cm4.h;cmsis_core/Include/core_cm7.h")

file(GLOB HEADERS_GK CONFIGURE_DEPENDS
	${CMAKE_CURRENT_SOURCE_DIR}/inc-gk/*.h
)
file(GLOB HEADERS_GK_SYS CONFIGURE_DEPENDS
	${CMAKE_CURRENT_SOURCE_DIR}/inc-gk/sys/*.h
)
file(GLOB HEADERS_GK_BITS CONFIGURE_DEPENDS
	${CMAKE_CURRENT_SOURCE_DIR}/inc-gk/bits/*.h
)
file(GLOB HEADERS_GK_ARPA CONFIGURE_DEPENDS
	${CMAKE_CURRENT_SOURCE_DIR}/inc-gk/arpa/*.h
)
file(GLOB HEADERS_GK_NET CONFIGURE_DEPENDS
	${CMAKE_CURRENT_SOURCE_DIR}/inc-gk/net/*.h
)
file(GLOB HEADERS_GK_NETINET CONFIGURE_DEPENDS
	${CMAKE_CURRENT_SOURCE_DIR}/inc-gk/netinet/*.h
)
file(GLOB HEADERS_GK_MACHINE CONFIGURE_DEPENDS
	${CMAKE_CURRENT_SOURCE_DIR}/inc-gk/machine/*.h
)
file(GLOB HEADERS_GK_USERLAND CONFIGURE_DEPENDS
	${CMAKE_CURRENT_SOURCE_DIR}/gk-userlandinterface/_netinet_in.h
	${CMAKE_CURRENT_SOURCE_DIR}/gk-userlandinterface/_sys_dirent.h
	${CMAKE_CURRENT_SOURCE_DIR}/gk-userlandinterface/_gk_gpu.h
	${CMAKE_CURRENT_SOURCE_DIR}/gk-userlandinterface/_gk_proccreate.h
	${CMAKE_CURRENT_SOURCE_DIR}/gk-userlandinterface/_gk_event.h
	${CMAKE_CURRENT_SOURCE_DIR}/gk-userlandinterface/_gk_scancodes.h
	${CMAKE_CURRENT_SOURCE_DIR}/gk-userlandinterface/_gk_memaddrs.h
)

install(FILES 
	${HEADERS_GK} DESTINATION include)
install(FILES 
	${HEADERS_GK_SYS} DESTINATION include/sys)
install(FILES 
	${HEADERS_GK_BITS} DESTINATION include/bits)
install(FILES 
	${HEADERS_GK_ARPA} DESTINATION include/arpa)
install(FILES 
	${HEADERS_GK_NET} DESTINATION include/net)
install(FILES 
	${HEADERS_GK_NETINET} DESTINATION include/netinet)
install(FILES
	${HEADERS_GK_MACHINE} DESTINATION include/machine)
install(FILES
	${HEADERS_GK_USERLAND} DESTINATION include)

add_library(pthread)

target_sources(pthread
	PRIVATE
	${SOURCES_PTHREAD}
)

target_include_directories(pthread
	PRIVATE
		"${CMAKE_CURRENT_SOURCE_DIR}/inc-pthread"
		"${CMAKE_CURRENT_SOURCE_DIR}/gk-userlandinterface"
		"${CMAKE_CURRENT_SOURCE_DIR}/src-pthread"
)
#set_target_properties(pthread PROPERTIES PUBLIC_HEADER "inc-pthread/pthread.h;inc-pthread/sched.h")

install(TARGETS gloss-gk pthread gk)


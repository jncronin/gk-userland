cmake_minimum_required(VERSION 3.11 FATAL_ERROR)

project(gloss-gk C ASM)

file(GLOB_RECURSE SOURCES_GLOSS CONFIGURE_DEPENDS
	${CMAKE_CURRENT_SOURCE_DIR}/src-gloss/*.c
)

file(GLOB_RECURSE SOURCES_PTHREAD CONFIGURE_DEPENDS
	${CMAKE_CURRENT_SOURCE_DIR}/src-pthread/*.c
)

add_library(gloss-gk)

target_sources(gloss-gk
	PRIVATE
	${SOURCES_GLOSS}
)

add_library(pthread)

target_sources(pthread
	PRIVATE
	${SOURCES_PTHREAD}
)

target_include_directories(pthread
	PRIVATE
		"${CMAKE_CURRENT_SOURCE_DIR}/inc-pthread"
)
set_target_properties(pthread PROPERTIES PUBLIC_HEADER "inc-pthread/pthread.h;inc-pthread/sched.h")

install(TARGETS gloss-gk pthread)


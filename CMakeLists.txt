cmake_minimum_required(VERSION 3.0)

set(TOPLEVEL_PROJECT FALSE)
if ("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_CURRENT_SOURCE_DIR}")
	set(TOPLEVEL_PROJECT TRUE)
endif ()

if (POLICY CMP0063)
	cmake_policy(SET CMP0063 NEW)
endif()

if (TOPLEVEL_PROJECT)
	option(DDSIMG_BUILD_SHARED_LIB "Build libddsimg as a shared library" TRUE)

	option(DDSIMG_BUILD_TESTS "Build tests" TRUE)

	option(DDSIMG_BUILD_EXAMPLES "Build examples" TRUE)
endif ()

project(ddsimg)

set(SOURCES
		include/ddsimg/ddsimg.h

		src/version.c src/memory.c src/memory.h src/context.c src/context.h src/image.c src/image.h src/dxtc.c src/dxtc.h src/util.c src/util.h)

if (DDSIMG_BUILD_SHARED_LIB)
	set(BUILD_TYPE SHARED)
else ()
	set(BUILD_TYPE STATIC)
endif ()

set(CMAKE_C_VISIBILITY_PRESET hidden)
add_library(ddsimg ${BUILD_TYPE} ${SOURCES})

target_include_directories(ddsimg PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(ddsimg PUBLIC ${CMAKE_CURRENT_BINARY_DIR})
target_include_directories(ddsimg PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

if ("${BUILD_TYPE}" STREQUAL "STATIC")
	target_compile_definitions(ddsimg PUBLIC DDSIMG_STATIC_DEFINE)
endif ()

include(GenerateExportHeader)

generate_export_header(ddsimg
	EXPORT_FILE_NAME ${CMAKE_CURRENT_BINARY_DIR}/ddsimg/ddsimg_export.h)

if (DDSIMG_BUILD_TESTS)
	add_subdirectory(test)
endif ()

if (DDSIMG_BUILD_EXAMPLES)
	add_subdirectory(examples)
endif()
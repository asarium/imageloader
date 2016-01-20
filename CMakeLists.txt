cmake_minimum_required(VERSION 3.1)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(TOPLEVEL_PROJECT FALSE)
if ("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_CURRENT_SOURCE_DIR}")
	set(TOPLEVEL_PROJECT TRUE)
endif ()

project(imageloader)

macro(library_option NAME DESCRIPTION DEFAULT_VAL)
	if (TOPLEVEL_PROJECT)
		option(${NAME} "${DESCRIPTION}" ${DEFAULT_VAL})
	else()
		if (NOT DEFINED ${NAME})
			set(${NAME} ${DEFAULT_VAL})
		endif()
	endif()
endmacro(library_option)

library_option(IMGLOADER_WITH_LIBDDSIMG "Build with DDS support by using libddsimg" TRUE)

library_option(IMGLOADER_WITH_PNG "Build with PNG support by using libpng" TRUE)

library_option(IMGLOADER_WITH_STB_IMAGE "Build a plugin for loading images using stb_image" TRUE)


library_option(IMGLOADER_BUILD_TESTS "Build tests for imageloader" TRUE)

library_option(IMGLOADER_BUILD_EXAMPLES "Build examples for imageloader" TRUE)

library_option(IMGLOADER_BUILD_CPP_API "Build the C++ API" TRUE)

add_subdirectory(src)

if (IMGLOADER_BUILD_CPP_API)
    add_subdirectory(cpp)
endif()

if(IMGLOADER_BUILD_TESTS)
	add_subdirectory(test)
endif()

if (IMGLOADER_BUILD_EXAMPLES)
	add_subdirectory(examples)
endif()

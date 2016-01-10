cmake_minimum_required(VERSION 3.1)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

project(imageloader)

option(WITH_LIBDDSIMG "Build with DDS support by using libddsimg" TRUE)

option(WITH_PNG "Build with PNG support by using libpng" TRUE)

option(IMGLOADER_BUILD_TESTS "Build tests for imageloader" TRUE)

add_subdirectory(src)

if(IMGLOADER_BUILD_TESTS)
	add_subdirectory(test)
endif()

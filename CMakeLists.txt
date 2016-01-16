cmake_minimum_required(VERSION 3.1)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

project(imageloader)

option(WITH_LIBDDSIMG "Build with DDS support by using libddsimg" TRUE)

option(WITH_PNG "Build with PNG support by using libpng" TRUE)

option(WITH_STB_IMAGE "Build a plugin for loading images using stb_image" TRUE)

option(WITH_PCX "Build a plugin for loading pcx plugins" TRUE)

option(IMGLOADER_BUILD_TESTS "Build tests for imageloader" TRUE)

option(IMGLOADER_BUILD_EXAMPLES "Build examples for imageloader" TRUE)

option(IMGLOADER_BUILD_CPP_API "Build the C++ API" TRUE)

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

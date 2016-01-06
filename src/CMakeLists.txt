
set(LOADER_HEADERS ../include/imageloader.h ../include/imageloader_plugin.h)

set(LOADER_SOURCES
	context.h context.c
	log.h log.c
	memory.h memory.c
	plugin.h plugin.c
	plugin_api.c
	image.h image.c
	version.c)

source_group("API Headers" FILES ${LOADER_HEADERS})

add_library(imageloader ${LOADER_HEADERS} ${LOADER_SOURCES})

target_include_directories(imageloader PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../include)
target_include_directories(imageloader PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

if (BUILD_SHARED_LIBS)
	target_compile_definitions(imageloader PUBLIC IMGLOAD_BUILDING_DLL)
	target_compile_definitions(imageloader PRIVATE IMGLOAD_COMPILING)
endif()
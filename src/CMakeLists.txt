
set(LOADER_HEADERS ../include/imageloader.h ../include/imageloader_plugin.h)

configure_file(project.h.in ${CMAKE_CURRENT_BINARY_DIR}/generated/project.h)

set(LOADER_SOURCES
        context.h context.c
        log.h log.c
        memory.h memory.c
        plugin.h plugin.c
        plugin_api.c
        image.h image.c
        version.c
        util.h
        format.c format.h
        ${CMAKE_CURRENT_BINARY_DIR}/generated/project.h
        packed.h)

source_group("API Headers" FILES ${LOADER_HEADERS})

add_library(imageloader ${LOADER_HEADERS} ${LOADER_SOURCES})
set_target_properties(imageloader PROPERTIES C_STANDARD 99)

target_include_directories(imageloader PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/../include")
target_include_directories(imageloader PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_BINARY_DIR}/generated")

if (MSVC)
    target_compile_definitions(imageloader PRIVATE _CRT_SECURE_NO_WARNINGS)
endif ()

if (BUILD_SHARED_LIBS)
    target_compile_definitions(imageloader PUBLIC IMGLOAD_BUILDING_DLL)
    target_compile_definitions(imageloader PRIVATE IMGLOAD_COMPILING)
endif ()

add_subdirectory(plugins)

if (WITH_LIBDDSIMG)
    target_link_libraries(imageloader PRIVATE plugin_ddsimg)
endif ()
if (WITH_PNG)
    target_link_libraries(imageloader PRIVATE plugin_png)
endif ()
if (WITH_STB_IMAGE)
    target_link_libraries(imageloader PRIVATE plugin_stb_image)
endif ()

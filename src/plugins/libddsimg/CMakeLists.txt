
message(STATUS "Building with libddsimg plugin")

set(DDSIMG_BUILD_SHARED_LIB FALSE)
set(DDSIMG_BUILD_TESTS FALSE)
set(DDSIMG_BUILD_EXAMPLES FALSE)

add_subdirectory(libddsimg)

set_target_properties(ddsimg PROPERTIES FOLDER "imageloader Plugins/Libs")

set(SOURCE_FILES "ddsimg.c" "ddsimg.h")

add_library(plugin_ddsimg STATIC "${SOURCE_FILES}")
set_target_properties (plugin_ddsimg PROPERTIES C_STANDARD 99)

set_target_properties(plugin_ddsimg PROPERTIES FOLDER "imageloader Plugins")

target_include_directories(plugin_ddsimg PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(plugin_ddsimg PRIVATE ddsimg imageloader)

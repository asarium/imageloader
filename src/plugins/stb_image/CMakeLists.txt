
message(STATUS "Building with stb_image plugin")
add_library(plugin_stb_image STATIC plugin_stb_image.c plugin_stb_image.h stb_image.c stb_image.h)
set_target_properties (plugin_stb_image PROPERTIES C_STANDARD 99)

set_target_properties(plugin_stb_image PROPERTIES FOLDER "imageloader Plugins")

target_include_directories(plugin_stb_image PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

target_link_libraries(plugin_stb_image PRIVATE imageloader)

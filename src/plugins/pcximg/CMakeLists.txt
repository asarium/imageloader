
add_library(plugin_pcx STATIC plugin_pcx.c plugin_pcx.h)
set_target_properties (plugin_pcx PROPERTIES C_STANDARD 99)

set_target_properties(plugin_pcx PROPERTIES FOLDER "imageloader Plugins")

target_include_directories(plugin_pcx PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

target_link_libraries(plugin_pcx PRIVATE imageloader)

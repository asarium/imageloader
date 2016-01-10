
# If there is a png target, assume it's a target building libpng
if (NOT TARGET png)
	find_package(ZLIB REQUIRED)
	find_package(PNG REQUIRED)
	
	add_library(png INTERFACE)
	
	target_include_directories(png INTERFACE ${ZLIB_INCLUDE_DIRS} ${PNG_INCLUDE_DIRS})
	target_compile_definitions(png INTERFACE ${PNG_DEFINITIONS})
	target_link_libraries(png INTERFACE ${ZLIB_LIBRARIES} ${PNG_LIBRARIES})
endif()

add_library(plugin_png STATIC plugin_png.c plugin_png.h)
set_target_properties (plugin_png PROPERTIES C_STANDARD 99)

set_target_properties(plugin_png PROPERTIES FOLDER "imageloader Plugins")

target_include_directories(plugin_png PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

target_link_libraries(plugin_png PRIVATE png imageloader)


add_executable(dump_mipmaps dump_mipmaps.c stb_image_write.h)
target_link_libraries(dump_mipmaps PRIVATE imageloader)

set_target_properties(dump_mipmaps PROPERTIES FOLDER "imageloader Examples")

if (MSVC)
	target_compile_definitions(dump_mipmaps PRIVATE _CRT_SECURE_NO_WARNINGS)
endif()

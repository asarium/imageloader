
if (IMGLOADER_WITH_LIBDDSIMG)
	add_subdirectory(libddsimg)
endif()

if (IMGLOADER_WITH_PNG)
	add_subdirectory(png)
endif()

if (IMGLOADER_WITH_STB_IMAGE)
	add_subdirectory(stb_image)
endif()

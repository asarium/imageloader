
add_executable(dump_metadata dump_metadata.c)
target_link_libraries(dump_metadata PRIVATE ddsimg)

add_executable(dump_mipmaps dump_mipmaps.c)
target_link_libraries(dump_mipmaps PRIVATE ddsimg)
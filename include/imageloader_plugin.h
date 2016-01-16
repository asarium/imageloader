#ifndef IMAGELOADER_IMAGELOADER_PLUGIN_H
#define IMAGELOADER_IMAGELOADER_PLUGIN_H
#pragma once

#include <imageloader.h>

#ifdef __cplusplus
extern "C"
{
#endif

void* IMGLOAD_API imgload_plugin_realloc(ImgloadPlugin plugin, void* ptr, size_t size);
void IMGLOAD_API imgload_plugin_free(ImgloadPlugin plugin, void* ptr);

void IMGLOAD_API imgload_plugin_log(ImgloadPlugin plugin, ImgloadLogLevel level, const char* format, ...);

void IMGLOAD_API imgload_plugin_set_data(ImgloadPlugin plugin, void* data);
void* IMGLOAD_API imgload_plugin_get_data(ImgloadPlugin plugin);

void IMGLOAD_API imgload_plugin_set_info(ImgloadPlugin plugin, const char* id, const char* name, const char* description);


typedef void(IMGLOAD_CALLBACK *ImgloadPluginDeinitFunc)(ImgloadPlugin plugin);

typedef int(IMGLOAD_CALLBACK *ImgloadPluginProbeFunc)(ImgloadPlugin plugin, ImgloadImage img);

typedef ImgloadErrorCode(IMGLOAD_CALLBACK *ImgloadPluginImageFunc)(ImgloadPlugin plugin, ImgloadImage img);

typedef ImgloadErrorCode(IMGLOAD_CALLBACK *ImgloadPluginDecompressData)(ImgloadPlugin plugin, ImgloadImage img, size_t subimage, size_t mipmap);


void IMGLOAD_API imgload_plugin_callback_deinit(ImgloadPlugin plugin, ImgloadPluginDeinitFunc func);

void IMGLOAD_API imgload_plugin_callback_probe(ImgloadPlugin plugin, ImgloadPluginProbeFunc func);

void IMGLOAD_API imgload_plugin_callback_init_image(ImgloadPlugin plugin, ImgloadPluginImageFunc func);

void IMGLOAD_API imgload_plugin_callback_deinit_image(ImgloadPlugin plugin, ImgloadPluginImageFunc func);

void IMGLOAD_API imgload_plugin_callback_read_data(ImgloadPlugin plugin, ImgloadPluginImageFunc func);

void IMGLOAD_API imgload_plugin_callback_decompress_data(ImgloadPlugin plugin, ImgloadPluginDecompressData func);

size_t IMGLOAD_API imgload_plugin_image_read(ImgloadImage img, uint8_t* buf, size_t size);
int64_t IMGLOAD_API imgload_plugin_image_seek(ImgloadImage img, int64_t offset, int whence);

void IMGLOAD_API imgload_plugin_image_set_data(ImgloadImage img, void* data);
void* IMGLOAD_API imgload_plugin_image_get_data(ImgloadImage img);

ImgloadErrorCode IMGLOAD_API imgload_plugin_image_set_num_frames(ImgloadImage img, size_t num_frames);

void IMGLOAD_API imgload_plugin_image_set_data_type(ImgloadImage img, ImgloadFormat format, ImgloadCompression compreesion);

ImgloadErrorCode IMGLOAD_API imgload_plugin_image_set_property(ImgloadImage img, size_t subimage,
    ImgloadProperty prop, ImgloadPropertyType type, void* val);

ImgloadErrorCode IMGLOAD_API imgload_plugin_image_set_num_mipmaps(ImgloadImage img, size_t subimage, size_t mipmaps);

ImgloadErrorCode IMGLOAD_API imgload_plugin_image_set_compressed_data(ImgloadImage img, size_t subimage, size_t mipmap,
    ImgloadImageData* data, int transfer_ownership);

ImgloadErrorCode IMGLOAD_API imgload_plugin_image_set_image_data(ImgloadImage img, size_t subimage, size_t mipmap,
    ImgloadImageData* data, int transfer_ownership);

#ifdef __cplusplus
}
#endif

#endif //IMAGELOADER_IMAGELOADER_PLUGIN_H

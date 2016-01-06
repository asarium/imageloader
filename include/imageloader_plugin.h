#ifndef IMAGELOADER_IMAGELOADER_PLUGIN_H
#define IMAGELOADER_IMAGELOADER_PLUGIN_H
#pragma once

#include <imageloader.h>

#ifdef __cplusplus
extern "C"
{
#endif

size_t IMGLOAD_API imgload_plugin_read(ImgloadPlugin plugin, ImgloadImage img, uint8_t* buf, size_t size);

int64_t IMGLOAD_API imgload_plugin_seek(ImgloadPlugin plugin, ImgloadImage img, int64_t offset, int whence);


void* IMGLOAD_API imgload_plugin_realloc(ImgloadPlugin plugin, void* ptr, size_t size);

void IMGLOAD_API imgload_plugin_free(ImgloadPlugin plugin, void* ptr);

void IMGLOAD_API imgload_plugin_log(ImgloadPlugin plugin, ImgloadLogLevel level, const char* format, ...);

void IMGLOAD_API imgload_plugin_set_data(ImgloadPlugin plugin, void* data);
void* IMGLOAD_API imgload_plugin_get_data(ImgloadPlugin plugin);

void IMGLOAD_API imgload_plugin_set_info(ImgloadPlugin plugin, const char* id, const char* name, const char* description);

typedef void(IMGLOAD_CALLBACK *ImgloadPluginDeinitFunc)(ImgloadPlugin plugin);
void IMGLOAD_API imgload_plugin_callback_deinit(ImgloadPlugin plugin, ImgloadPluginDeinitFunc func);

typedef ImgloadErrorCode(IMGLOAD_CALLBACK *ImgloadPluginProbeFunc)(ImgloadPlugin plugin, int* probe_successful_out);
void IMGLOAD_API imgload_plugin_callback_probe(ImgloadPlugin plugin, ImgloadPluginProbeFunc func);

typedef ImgloadErrorCode(IMGLOAD_CALLBACK *ImgloadPluginImageFunc)(ImgloadPlugin plugin, ImgloadImage img);
void IMGLOAD_API imgload_plugin_callback_init_image(ImgloadPlugin plugin, ImgloadPluginImageFunc func);

void IMGLOAD_API imgload_plugin_callback_deinit_image(ImgloadPlugin plugin, ImgloadPluginImageFunc func);

#ifdef __cplusplus
}
#endif

#endif //IMAGELOADER_IMAGELOADER_PLUGIN_H

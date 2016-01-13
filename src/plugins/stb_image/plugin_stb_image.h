
#pragma once

#include <imageloader.h>

#ifdef __cplusplus
extern "C"
{
#endif

ImgloadErrorCode IMGLOAD_CALLBACK stb_image_plugin_loader(ImgloadPlugin plugin, void* parameter);

#ifdef __cplusplus
}
#endif


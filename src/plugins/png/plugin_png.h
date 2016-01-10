#ifndef PLUGIN_PNG_H
#define PLUGIN_PNG_H
#pragma once

#include <imageloader.h>

#ifdef __cplusplus
extern "C"
{
#endif

ImgloadErrorCode IMGLOAD_CALLBACK png_plugin_loader(ImgloadPlugin plugin, void* parameter);

#ifdef __cplusplus
}
#endif

#endif //PLUGIN_PNG_H

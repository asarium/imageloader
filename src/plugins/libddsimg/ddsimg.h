#ifndef PLUGIN_DDSIMG_H
#define PLUGIN_DDSIMG_H
#pragma once

#include <imageloader.h>

#ifdef __cplusplus
extern "C"
{
#endif

ImgloadErrorCode IMGLOAD_CALLBACK ddsimg_plugin_loader(ImgloadPlugin plugin, void* parameter);

#ifdef __cplusplus
}
#endif

#endif //PLUGIN_DDSIMG_H

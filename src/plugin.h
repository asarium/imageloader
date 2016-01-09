#ifndef IMAGELOADER_PLUGIN_H
#define IMAGELOADER_PLUGIN_H
#pragma once

#include <imageloader.h>
#include <imageloader_plugin.h>

struct ImgloadPluginImpl
{
    struct ImgloadPluginImpl* prev;
    struct ImgloadPluginImpl* next;

    ImgloadContext context;

    void* plugin_data;

    struct
    {
        const char* id;
        const char* name;
        const char* description;
    } info;

    struct
    {
        ImgloadPluginDeinitFunc deinit;
        ImgloadPluginProbeFunc probe;

        ImgloadPluginImageFunc init_image;
        ImgloadPluginImageFunc deinit_image;

        ImgloadPluginImageFunc read_image;
        ImgloadPluginDecompressData decompress_data;
    } funcs;
};

ImgloadErrorCode plugin_init(ImgloadContext ctx, ImgloadPluginLoader loader, void* param, ImgloadPlugin* plugin_out);

void plugin_free(ImgloadPlugin plugin);

#endif //IMAGELOADER_PLUGIN_H

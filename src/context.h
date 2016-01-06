#ifndef IMAGELOADER_CONTEXT_H
#define IMAGELOADER_CONTEXT_H
#pragma once

#include <imageloader.h>

#include "plugin.h"

typedef struct ImgloadContextImpl
{
    struct
    {
        ImgloadMemoryAllocator allocator;
        void* ud;
    } mem;

    struct
    {
        ImgloadPluginImpl* head;
        ImgloadPluginImpl* tail;
    } plugins;

    struct
    {
        ImgloadLogHandler handler;
        void* ud;
    } log;

    const char* last_error;
} ImgloadContextImpl;

void ctx_set_last_error(ImgloadContext ctx, const char* err);

#endif //IMAGELOADER_CONTEXT_H

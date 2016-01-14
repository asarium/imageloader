#ifndef IMAGELOADER_CONTEXT_H
#define IMAGELOADER_CONTEXT_H
#pragma once

#include <imageloader.h>

#include "plugin.h"

struct ImgloadContextImpl
{
    struct
    {
        ImgloadMemoryAllocator allocator;
        void* ud;
    } mem;

    ImgloadContextFlags flags;

    struct
    {
        ImgloadPlugin head;
        ImgloadPlugin tail;
    } plugins;

    struct
    {
        ImgloadLogHandler handler;
        void* ud;
    } log;
};

#endif //IMAGELOADER_CONTEXT_H

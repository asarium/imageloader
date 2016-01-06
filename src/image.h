#ifndef IMAGELOADER_IMAGE_H
#define IMAGELOADER_IMAGE_H
#pragma once

#include <imageloader.h>

typedef struct ImgloadImageImpl
{
    ImgloadContext context;
    ImgloadPlugin plugin;

    struct
    {
        ImgloadCustomIO funcs;
        void* ud;
    } io;
} ImgloadImageImpl;

size_t IMGLOAD_API image_io_read(ImgloadImage img, uint8_t* buf, size_t size);

int64_t IMGLOAD_API image_io_seek(ImgloadImage img, int64_t offset, int whence);


#endif //IMAGELOADER_IMAGE_H

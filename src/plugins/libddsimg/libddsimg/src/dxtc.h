
#ifndef DDSIMG_DXTC_H
#define DDSIMG_DXTC_H
#pragma once

#include <stddef.h>

#include <ddsimg/ddsimg.h>
#include "image.h"

typedef struct Color8888
{
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t a;
} Color8888;

size_t dxtc_get_data_size(DDSCompressedFormat_t format, uint32_t width, uint32_t height);

DDSErrorCode dxtc_decrompress(DDSImage* img, MipmapData* compressed, MipmapData* decompressed);

#endif //DDSIMG_DXTC_H

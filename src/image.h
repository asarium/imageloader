#ifndef IMAGELOADER_IMAGE_H
#define IMAGELOADER_IMAGE_H
#pragma once

#include <imageloader.h>

#include <stdbool.h>

typedef struct
{
    bool initialized;
    ImgloadPropertyType type;
    union
    {
        uint32_t uint32;
        int32_t int32;
        float float_val;
        double double_val;
        char* str;
        void* complex;
    } value;
} PropertyValue;

typedef struct
{
    ImgloadImageData image;
    bool has_data;
} MipmapData;

typedef struct
{
    MipmapData compressed;
    MipmapData raw;
} Mipmap;

typedef struct
{
    PropertyValue properties[IMGLOAD_PROPERTY_MAX];

    size_t n_mipmaps;
    Mipmap* mipmaps;
} ImageFrame;

struct ImgloadImageImpl
{
    ImgloadContext context;

    ImgloadPlugin plugin;
    void* plugin_data;

    bool data_format_initialized;
    ImgloadFormat plugin_data_format; //!< The format of the data the plugin provides

    ImgloadFormat data_format; //!< The actual format of the image data, can be different if changed after loading data

    bool compression_initialized;
    ImgloadCompression compression;

    struct
    {
        bool do_convert;
        ImgloadFormat requested;
        uint64_t param;
    } conv;

    struct
    {
        ImgloadIO funcs;
        void* ud;
    } io;

    ImageFrame* frames;
    size_t n_frames;
};

size_t IMGLOAD_API image_io_read(ImgloadImage img, uint8_t* buf, size_t size);

int64_t IMGLOAD_API image_io_seek(ImgloadImage img, int64_t offset, int whence);

ImgloadErrorCode image_allocate_frames(ImgloadImage img, size_t num_frames);

ImgloadErrorCode image_allocate_mipmaps(ImgloadImage img, size_t subframe, size_t mipmaps);

ImgloadErrorCode image_set_compressed_data(ImgloadImage img, size_t subframe, size_t mipmap,
                                            ImgloadImageData* data, bool transfer_ownership);

ImgloadErrorCode image_set_data(ImgloadImage img, size_t subframe, size_t mipmap,
                                            ImgloadImageData* data, bool transfer_ownership);

#endif //IMAGELOADER_IMAGE_H


#include "plugin_stb_image.h"

#include <imageloader_plugin.h>

#define STBI_NO_HDR
#define STBI_NO_LINEAR
#define STBI_NO_STDIO
#include "stb_image.h"

#include <stdio.h>
#include <string.h>

static int stb_read(void* user, char* data, int size)
{
    ImgloadImage img = (ImgloadImage)user;
    
    return (int)imgload_plugin_image_read(img, (uint8_t*)data, (size_t)size);
}

static void stb_skip(void* user, int n)
{
    ImgloadImage img = (ImgloadImage)user;

    imgload_plugin_image_seek(img, (int)n, SEEK_CUR);
}

static int stb_eof(void* user)
{
    ImgloadImage img = (ImgloadImage)user;

    // Save the current position
    int64_t curr = imgload_plugin_image_seek(img, 0, SEEK_CUR);

    int64_t end = imgload_plugin_image_seek(img, 0, SEEK_END);

    int eof = curr >= end;

    // Restore previous position
    imgload_plugin_image_seek(img, curr, SEEK_SET);

    return eof;
}

static int IMGLOAD_CALLBACK stb_image_probe(ImgloadPlugin plugin, ImgloadImage img)
{
    stbi_io_callbacks callbacks;
    callbacks.read = stb_read;
    callbacks.skip = stb_skip;
    callbacks.eof = stb_eof;

    int width, height, components;

    int ret = stbi_info_from_callbacks(&callbacks, img, &width, &height, &components);

    return ret != 0;
}

static ImgloadErrorCode IMGLOAD_CALLBACK stb_image_init_image(ImgloadPlugin plugin, ImgloadImage img)
{
    stbi_io_callbacks callbacks;
    callbacks.read = stb_read;
    callbacks.skip = stb_skip;
    callbacks.eof = stb_eof;

    int width, height, components;

    imgload_plugin_image_seek(img, 0, SEEK_SET);

    int ret = stbi_info_from_callbacks(&callbacks, img, &width, &height, &components);

    if (!ret)
    {
        return IMGLOAD_ERR_PLUGIN_ERROR;
    }

    imgload_plugin_image_set_num_frames(img, 1);
    imgload_plugin_image_set_num_mipmaps(img, 0, 1);

    uint32_t uwidth, uheight;
    uwidth = (uint32_t)width;
    uheight = (uint32_t)height;

    imgload_plugin_image_set_property(img, 0, IMGLOAD_PROPERTY_WIDTH, IMGLOAD_PROPERTY_TYPE_UINT32, &uwidth);
    imgload_plugin_image_set_property(img, 0, IMGLOAD_PROPERTY_HEIGHT, IMGLOAD_PROPERTY_TYPE_UINT32, &uheight);

    uint32_t one = 1;
    imgload_plugin_image_set_property(img, 0, IMGLOAD_PROPERTY_DEPTH, IMGLOAD_PROPERTY_TYPE_UINT32, &one);

    ImgloadFormat format;
    switch(components)
    {
    case STBI_grey:
        format = IMGLOAD_FORMAT_GRAY8;
        break;
    case STBI_rgb:
        format = IMGLOAD_FORMAT_R8G8B8;
        break;
    case STBI_rgb_alpha:
        format = IMGLOAD_FORMAT_R8G8B8A8;
        break;
    default:
        return IMGLOAD_ERR_UNSUPPORTED_FORMAT;
    }
    imgload_plugin_image_set_data_type(img, format, IMGLOAD_COMPRESSION_NONE);

    return IMGLOAD_ERR_NO_ERROR;
}

static ImgloadErrorCode IMGLOAD_CALLBACK png_read_data(ImgloadPlugin plugin, ImgloadImage img)
{
    stbi_io_callbacks callbacks;
    callbacks.read = stb_read;
    callbacks.skip = stb_skip;
    callbacks.eof = stb_eof;


    imgload_plugin_image_seek(img, 0, SEEK_SET);

    int width, height, components;
    stbi_uc* ret = stbi_load_from_callbacks(&callbacks, img, &width, &height, &components, STBI_default);

    if (!ret)
    {
        return IMGLOAD_ERR_PLUGIN_ERROR;
    }

    int bpp;
    switch (components)
    {
    case STBI_grey:
        bpp = 1;
        break;
    case STBI_rgb:
        bpp = 3;
        break;
    case STBI_rgb_alpha:
        bpp = 4;
        break;
    default:
        return IMGLOAD_ERR_UNSUPPORTED_FORMAT;
    }

    size_t stride = width * bpp;
    size_t total_size = height * stride;

    uint8_t* buffer = (uint8_t*)imgload_plugin_realloc(plugin, NULL, total_size);
    if (!buffer)
    {
        stbi_image_free(ret);
        return IMGLOAD_ERR_OUT_OF_MEMORY;
    }

    // Copy data to our buffer
    memcpy(buffer, ret, total_size);

    stbi_image_free(ret);

    ImgloadImageData data;
    data.width = width;
    data.height = height;
    data.depth = 1;

    data.stride = stride;
    data.data_size = total_size;
    data.data = buffer;

    imgload_plugin_image_set_image_data(img, 0, 0, &data, 1);

    return IMGLOAD_ERR_NO_ERROR;
}

ImgloadErrorCode IMGLOAD_CALLBACK stb_image_plugin_loader(ImgloadPlugin plugin, void* parameter)
{
    imgload_plugin_set_info(plugin, "stb_image", "stb_image Plugin", "Loads images using stb_image");

    imgload_plugin_callback_probe(plugin, stb_image_probe);
    imgload_plugin_callback_init_image(plugin, stb_image_init_image);

    imgload_plugin_callback_read_data(plugin, png_read_data);

    return IMGLOAD_ERR_NO_ERROR;
}
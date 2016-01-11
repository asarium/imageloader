
#include "ddsimg.h"

#include <imageloader_plugin.h>

#include <ddsimg/ddsimg.h>

static void* DDSIMG_CALLBACK plugin_realloc(void* ud, void* mem, size_t size)
{
    ImgloadPlugin plugin = (ImgloadPlugin)ud;

    return imgload_plugin_realloc(plugin, mem, size);
}

static void DDSIMG_CALLBACK plugin_free(void* ud, void* mem)
{
    ImgloadPlugin plugin = (ImgloadPlugin)ud;

imgload_plugin_free(plugin, mem);
}

static size_t DDSIMG_CALLBACK plugin_img_read(void* ud, uint8_t* buf, size_t size)
{
    return imgload_plugin_image_read((ImgloadImage)ud, buf, size);
}

static int64_t DDSIMG_CALLBACK plugin_img_seek(void* ud, int64_t offset, int whence)
{
    return imgload_plugin_image_seek((ImgloadImage)ud, offset, whence);
}


static ImgloadErrorCode convert_error(DDSErrorCode err)
{
    switch(err)
    {
    case DDSIMG_ERR_NO_ERROR:
        return IMGLOAD_ERR_NO_ERROR;
    case DDSIMG_ERR_OUT_OF_MEMORY:
        return IMGLOAD_ERR_OUT_OF_MEMORY;
    case DDSIMG_ERR_FORMAT_UNKNOWN:
    case DDSIMG_ERR_NOT_SUPPORTED:
        return IMGLOAD_ERR_UNSUPPORTED_FORMAT;
    case DDSIMG_ERR_FILE_INVALID:
    case DDSIMG_ERR_FILE_SIZE_WRONG:
        return IMGLOAD_ERR_FILE_INVALID;
    case DDSIMG_ERR_NO_DATA:
        return IMGLOAD_ERR_NO_DATA;
    default:
        return IMGLOAD_ERR_PLUGIN_ERROR;
    }
}


static void IMGLOAD_CALLBACK plugin_deinit(ImgloadPlugin plugin)
{
    DDSContext* ctx = (DDSContext*)imgload_plugin_get_data(plugin);
    if (ddsimg_context_free(&ctx) != DDSIMG_ERR_NO_ERROR)
    {
        imgload_plugin_log(plugin, IMGLOAD_LOG_ERROR, "Failed to deallocate libddsimg context!");
    }
}

static int IMGLOAD_CALLBACK plugin_probe(ImgloadPlugin plugin, ImgloadImage img)
{
    uint8_t magic[4];
    if (imgload_plugin_image_read(img, magic, 4) != 4)
    {
        return 0;
    }

    // Check for 'DDS ' magic header
    if (magic[0] != 'D')
    {
        return 0;
    }
    if (magic[1] != 'D')
    {
        return 0;
    }
    if (magic[2] != 'S')
    {
        return 0;
    }
    if (magic[3] != ' ')
    {
        return 0;
    }

    // Right header present
    return 1;
}

static ImgloadCompression convert_compression(uint32_t dds_comp)
{
    switch (dds_comp)
    {
    case DDSIMG_COM_DXT1:
        return IMGLOAD_COMPRESSION_DXT1;
    case DDSIMG_COM_DXT2:
        return IMGLOAD_COMPRESSION_DXT2;
    case DDSIMG_COM_DXT3:
        return IMGLOAD_COMPRESSION_DXT3;
    case DDSIMG_COM_DXT4:
        return IMGLOAD_COMPRESSION_DXT4;
    case DDSIMG_COM_DXT5:
        return IMGLOAD_COMPRESSION_DXT5;
    default:
        return IMGLOAD_COMPRESSION_NONE;
    }
}

static ImgloadFormat convert_format(uint32_t dds_format)
{
    switch(dds_format)
    {
    case DDSIMG_FORMAT_R8G8B8A8:
        return IMGLOAD_FORMAT_R8G8B8A8;
    default:
        return IMGLOAD_FORMAT_R8G8B8A8;
    }
}

static ImgloadErrorCode IMGLOAD_CALLBACK plugin_init_image(ImgloadPlugin plugin, ImgloadImage img)
{
    DDSIOFunctions io;
    io.read = plugin_img_read;
    io.seek = plugin_img_seek;

    DDSContext* ctx = (DDSContext*)imgload_plugin_get_data(plugin);

    DDSImage* dds_img = NULL;
    DDSErrorCode err = ddsimg_image_alloc(ctx, &dds_img, &io, img);

    if (err != DDSIMG_ERR_NO_ERROR)
    {
        switch (err)
        {
        case DDSIMG_ERR_OUT_OF_MEMORY:
            return IMGLOAD_ERR_OUT_OF_MEMORY;
        default:
            return IMGLOAD_ERR_PLUGIN_ERROR;
        }
    }

    err = ddsimg_image_read_header(dds_img);
    if (err != DDSIMG_ERR_NO_ERROR)
    {
        ddsimg_image_free(&dds_img);
        return IMGLOAD_ERR_PLUGIN_ERROR;
    }

    uint32_t subimages;
    err = ddsimg_image_get_num_subimages(dds_img, &subimages);
    if (err != DDSIMG_ERR_NO_ERROR)
    {
        imgload_plugin_log(plugin, IMGLOAD_LOG_ERROR, "Failed to get number if subimages of DDS image!");
        ddsimg_image_free(&dds_img);
        return IMGLOAD_ERR_PLUGIN_ERROR;
    }

    if (imgload_plugin_image_set_num_frames(img, (size_t)subimages) != IMGLOAD_ERR_NO_ERROR)
    {
        imgload_plugin_log(plugin, IMGLOAD_LOG_ERROR, "Failed to set number of subimages!");
        ddsimg_image_free(&dds_img);
        return IMGLOAD_ERR_PLUGIN_ERROR;
    }

    uint32_t width, height, depth;
    if (ddsimg_image_get_size(dds_img, &width, &height, &depth) != IMGLOAD_ERR_NO_ERROR)
    {
        imgload_plugin_log(plugin, IMGLOAD_LOG_ERROR, "Failed to get image size!");
        ddsimg_image_free(&dds_img);
        return IMGLOAD_ERR_PLUGIN_ERROR;
    }

    uint32_t compression, format;
    ddsimg_image_get_compression(dds_img, &compression);
    ddsimg_image_get_format(dds_img, &format);

    uint32_t mipmaps;
    ddsimg_image_get_num_mipmaps(dds_img, &mipmaps);

    imgload_plugin_image_set_data_type(img, convert_format(format), convert_compression(compression));

    for (uint32_t i = 0; i < subimages; ++i)
    {
        imgload_plugin_image_set_property(img, (size_t)i, IMGLOAD_PROPERTY_WIDTH, IMGLOAD_PROPERTY_TYPE_UINT32, &width);
        imgload_plugin_image_set_property(img, (size_t)i, IMGLOAD_PROPERTY_HEIGHT, IMGLOAD_PROPERTY_TYPE_UINT32, &height);
        imgload_plugin_image_set_property(img, (size_t)i, IMGLOAD_PROPERTY_DEPTH, IMGLOAD_PROPERTY_TYPE_UINT32, &depth);

        imgload_plugin_image_set_num_mipmaps(img, (size_t)i, (size_t)mipmaps);
    }

    imgload_plugin_image_set_data(img, (void*)dds_img);

    return IMGLOAD_ERR_NO_ERROR;
}

static ImgloadErrorCode IMGLOAD_CALLBACK plugin_read_data(ImgloadPlugin plugin, ImgloadImage img)
{
    DDSImage* dds_img = (DDSImage*)imgload_plugin_image_get_data(img);

    DDSErrorCode err = ddsimg_image_read_data(dds_img);

    switch(err)
    {
    case DDSIMG_ERR_NO_ERROR:
        break;
    case DDSIMG_ERR_OUT_OF_MEMORY:
        return IMGLOAD_ERR_OUT_OF_MEMORY;
    default:
        return IMGLOAD_ERR_PLUGIN_ERROR;
    }

    // Reading was successful
    uint32_t subimages, mipmaps;
    ddsimg_image_get_num_subimages(dds_img, &subimages);
    ddsimg_image_get_num_mipmaps(dds_img, &mipmaps);

    for (uint32_t i = 0; i < subimages; ++i)
    {
        for (uint32_t j = 0; j < mipmaps; ++j)
        {
            MipmapData data;
            err = ddsimg_image_get_compressed_data(dds_img, i, j, &data);
            if (err == DDSIMG_ERR_NO_ERROR)
            {
                ImgloadImageData new_data;

                new_data.width = (size_t)data.width;
                new_data.height = (size_t)data.height;
                new_data.depth = (size_t)data.depth;

                new_data.stride = new_data.width * 4; // Format is always 32 bit RGBA
                new_data.data_size = data.data_size;
                new_data.data = data.data;

                imgload_plugin_image_set_compressed_data(img, (size_t)i, (size_t)j, &new_data, 0);
            }
        }
    }

    return IMGLOAD_ERR_NO_ERROR;
}

static ImgloadErrorCode IMGLOAD_CALLBACK plugin_decompress_data(ImgloadPlugin plugin, ImgloadImage img, size_t subimage, size_t mipmap)
{
    DDSImage* dds_img = imgload_plugin_image_get_data(img);

    MipmapData data;
    DDSErrorCode err = ddsimg_image_get_decompressed_data(dds_img, (uint32_t)subimage, (uint32_t)mipmap, &data);

    if (err != DDSIMG_ERR_NO_ERROR)
    {
        return convert_error(err);
    }

    ImgloadImageData image_data;

    image_data.width = (size_t)data.width;
    image_data.height = (size_t)data.height;
    image_data.depth = (size_t)data.depth;

    image_data.data_size = data.data_size;
    image_data.data = data.data;

    imgload_plugin_image_set_image_data(img, subimage, mipmap, &image_data, 0);

    return IMGLOAD_ERR_NO_ERROR;
}

static ImgloadErrorCode IMGLOAD_CALLBACK plugin_deinit_image(ImgloadPlugin plugin, ImgloadImage img)
{
    DDSImage* dds_img = imgload_plugin_image_get_data(img);

    if (ddsimg_image_free(&dds_img) != DDSIMG_ERR_NO_ERROR)
    {
        return IMGLOAD_ERR_PLUGIN_ERROR;
    }

    return IMGLOAD_ERR_NO_ERROR;
}


ImgloadErrorCode IMGLOAD_CALLBACK ddsimg_plugin_loader(ImgloadPlugin plugin, void* parameter)
{
    DDSContext* ctx;
    DDSMemoryFunctions mem_funcs;
    mem_funcs.realloc = plugin_realloc;
    mem_funcs.free = plugin_free;

    imgload_plugin_set_info(plugin, "ddsimg", "libddsimg Plugin", "Parses DDS files using libddsimg");

    DDSErrorCode err = ddsimg_context_alloc(&ctx, &mem_funcs, (void*)plugin);

    if (err != DDSIMG_ERR_NO_ERROR)
    {
        if (err == DDSIMG_ERR_OUT_OF_MEMORY)
        {
            return IMGLOAD_ERR_OUT_OF_MEMORY;
        }

        return IMGLOAD_ERR_PLUGIN_ERROR;
    }

    imgload_plugin_set_data(plugin, (void*)ctx);

    imgload_plugin_callback_deinit(plugin, plugin_deinit);
    imgload_plugin_callback_probe(plugin, plugin_probe);

    imgload_plugin_callback_init_image(plugin, plugin_init_image);
    imgload_plugin_callback_deinit_image(plugin, plugin_deinit_image);

    imgload_plugin_callback_read_data(plugin, plugin_read_data);
    imgload_plugin_callback_decompress_data(plugin, plugin_decompress_data);

    return IMGLOAD_ERR_NO_ERROR;
}

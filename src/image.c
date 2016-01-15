#include "image.h"
#include "memory.h"
#include "plugin.h"
#include "context.h"
#include "log.h"

#include <string.h>
#include <assert.h>
#include <stdio.h>

static bool validate_image(ImgloadImage img)
{
    if (!img->data_format_initialized || !img->compression_initialized)
    {
        print_to_log(img->context, IMGLOAD_LOG_ERROR, "Data format or compression not initialized after image load!\n");
        return false;
    }

    return true;
}

ImgloadErrorCode IMGLOAD_API imgload_image_init(ImgloadContext ctx, ImgloadImage* image, ImgloadIO* io,
                                                   void* io_ud)
{
    assert(ctx != NULL);
    assert(image != NULL);

    ImgloadImage img = mem_reallocz(ctx, NULL, sizeof(struct ImgloadImageImpl));
    if (img == NULL)
    {
        return IMGLOAD_ERR_OUT_OF_MEMORY;
    }

    img->context = ctx;

    img->io.funcs = *io;
    img->io.ud = io_ud;

    ImgloadPlugin current = img->context->plugins.head;
    while (current != NULL)
    {
        if (current->funcs.probe(current, img))
        {
            // Found the right plugin, now initialize the plugin for this image
            ImgloadErrorCode err = current->funcs.init_image(current, img);

            if (err != IMGLOAD_ERR_NO_ERROR)
            {
                imgload_image_free(img);
                return err;
            }
            if (!validate_image(img))
            {
                imgload_image_free(img);
                return IMGLOAD_ERR_PLUGIN_ERROR;
            }
            img->plugin = current;

            *image = img;

            return IMGLOAD_ERR_NO_ERROR;
        }

        image_io_seek(img, 0, SEEK_SET);
        current = current->next;
    }

    // Unsupported format
    mem_free(ctx, img);
    return IMGLOAD_ERR_UNSUPPORTED_FORMAT;
}

size_t IMGLOAD_API imgload_image_num_subimages(ImgloadImage img)
{
    assert(img != NULL);

    return img->n_frames;
}

ImgloadErrorCode IMGLOAD_API imgload_image_get_property(ImgloadImage img, size_t subimage, ImgloadProperty prop,
    ImgloadPropertyType type, void* val_out)
{
    assert(img != NULL);

    if (subimage >= img->n_frames)
    {
        return IMGLOAD_ERR_OUT_OF_RANGE;
    }

    PropertyValue* property = &img->frames[subimage].properties[prop];
    if (!property->initialized)
    {
        return IMGLOAD_ERR_NO_DATA;
    }

    if (type != property->type)
    {
        return IMGLOAD_ERR_WRONG_TYPE;
    }

    switch(type)
    {
    case IMGLOAD_PROPERTY_TYPE_UINT32:
        *(uint32_t*)val_out = property->value.uint32;
        break;
    case IMGLOAD_PROPERTY_TYPE_INT32:
        *(int32_t*)val_out = property->value.int32;
        break;
    case IMGLOAD_PROPERTY_TYPE_FLAOT:
        *(float*)val_out = property->value.float_val;
        break;
    case IMGLOAD_PROPERTY_TYPE_DOUBLE:
        *(double*)val_out = property->value.double_val;
        break;
    case IMGLOAD_PROPERTY_TYPE_STRING:
        *(const char**)val_out = property->value.str;
        break;
    case IMGLOAD_PROPERTY_TYPE_COMPLEX:
        *(void**)val_out = property->value.complex;
        break;
    }

    return IMGLOAD_ERR_NO_ERROR;
}

ImgloadFormat IMGLOAD_API imgload_image_data_format(ImgloadImage img)
{
    assert(img != NULL);
    assert(img->data_format_initialized);

    return img->data_format;
}

ImgloadCompression IMGLOAD_API imgload_image_compression(ImgloadImage img)
{
    assert(img != NULL);
    assert(img->compression_initialized);

    return img->compression;
}

ImgloadErrorCode IMGLOAD_API imgload_image_read_data(ImgloadImage img)
{
    assert(img != NULL);
    assert(img->plugin != NULL);

    if (img->plugin->funcs.read_image)
    {
        return img->plugin->funcs.read_image(img->plugin, img);
    }

    // No read function => data must have been initialized earlier
    return IMGLOAD_ERR_NO_ERROR;
}

size_t IMGLOAD_API imgload_image_num_mipmaps(ImgloadImage img, size_t subimage)
{
    assert(img != NULL);
    assert(subimage < img->n_frames);

    return img->frames[subimage].n_mipmaps;
}

ImgloadErrorCode IMGLOAD_API imgload_image_compressed_data(ImgloadImage img, size_t subimage, size_t mipmap, ImgloadImageData* data_out)
{
    assert(img != NULL);
    assert(subimage < img->n_frames);
    assert(mipmap < img->frames[subimage].n_mipmaps);

    if (img->compression == IMGLOAD_COMPRESSION_NONE)
    {
        return IMGLOAD_ERR_NO_DATA;
    }

    if (!img->frames[subimage].mipmaps[mipmap].compressed.has_data)
    {
        return IMGLOAD_ERR_NO_DATA;
    }

    // Everything seems to be alright
    *data_out = img->frames[subimage].mipmaps[mipmap].compressed.image;

    return IMGLOAD_ERR_NO_ERROR;
}

ImgloadErrorCode IMGLOAD_API imgload_image_data(ImgloadImage img, size_t subimage, size_t mipmap, ImgloadImageData* data_out)
{
    assert(img != NULL);
    assert(subimage < img->n_frames);
    assert(mipmap < img->frames[subimage].n_mipmaps);

    if (img->frames[subimage].mipmaps[mipmap].raw.has_data)
    {
        // Data is already present
        *data_out = img->frames[subimage].mipmaps[mipmap].raw.image;

        return IMGLOAD_ERR_NO_ERROR;
    }

    // Raw data is not available but the plugin could do lazy decompression
    if (img->plugin->funcs.decompress_data != NULL)
    {
        ImgloadErrorCode err = img->plugin->funcs.decompress_data(img->plugin, img, subimage, mipmap);

        if (err == IMGLOAD_ERR_NO_ERROR)
        {
            if (img->frames[subimage].mipmaps[mipmap].raw.has_data)
            {
                // Data has been loaded by plugin
                *data_out = img->frames[subimage].mipmaps[mipmap].raw.image;

                return IMGLOAD_ERR_NO_ERROR;
            }
        }
        else
        {
            return err;
        }
    }

    return IMGLOAD_ERR_NO_DATA;
}

static void free_mipmap_data(ImgloadContext ctx, MipmapData* data)
{
    // Make sure that memory is allocated and we actually need to free the memory
    if (data->image.data != NULL)
    {
        mem_free(ctx, data->image.data);
    }
}

ImgloadErrorCode IMGLOAD_API imgload_image_free(ImgloadImage image)
{
    assert(image != NULL);

    if (image->plugin)
    {
        // If there is a plugin registered, deinitialize it when freeing the image
        if (image->plugin->funcs.deinit_image != NULL)
        {
            image->plugin->funcs.deinit_image(image->plugin, image);
        }
    }

    for (size_t i = 0; i < image->n_frames; ++i)
    {
        ImageFrame* frame = &image->frames[i];
        PropertyValue* val = &frame->properties[i];
        if (val->initialized && val->type == IMGLOAD_PROPERTY_TYPE_STRING)
        {
            mem_free(image->context, val->value.str);
        }

        Mipmap* mipmaps = frame->mipmaps;
        if (mipmaps != NULL)
        {
            for (size_t mipmap = 0; mipmap < frame->n_mipmaps; ++mipmap)
            {
                free_mipmap_data(image->context, &mipmaps[mipmap].compressed);
                free_mipmap_data(image->context, &mipmaps[mipmap].raw);
            }

            mem_free(image->context, mipmaps);
        }
    }

    mem_free(image->context, image->frames);

    mem_free(image->context, image);

    return IMGLOAD_ERR_NO_ERROR;
}

size_t IMGLOAD_API image_io_read(ImgloadImage img, uint8_t* buf, size_t size)
{
    assert(img != NULL);

    return img->io.funcs.read(img->io.ud, buf, size);
}

int64_t IMGLOAD_API image_io_seek(ImgloadImage img, int64_t offset, int whence)
{
    assert(img != NULL);

    return img->io.funcs.seek(img->io.ud, offset, whence);
}

ImgloadErrorCode image_allocate_frames(ImgloadImage img, size_t num_frames)
{
    assert(img != NULL);

    if (num_frames <= img->n_frames)
    {
        img->n_frames = num_frames;
        // All frames already allocated
        return IMGLOAD_ERR_NO_ERROR;
    }

    ImageFrame* new_frames = (ImageFrame*)mem_reallocz(img->context, img->frames, num_frames * sizeof(*img->frames));
    if (new_frames == NULL)
    {
        return IMGLOAD_ERR_OUT_OF_MEMORY;
    }

    img->frames = new_frames;
    img->n_frames = num_frames;

    return IMGLOAD_ERR_NO_ERROR;
}

ImgloadErrorCode image_allocate_mipmaps(ImgloadImage img, size_t subframe, size_t mipmaps)
{
    assert(img != NULL);
    assert(subframe < img->n_frames);

    ImageFrame* frame = &img->frames[subframe];
    if (mipmaps <= frame->n_mipmaps)
    {
        frame->n_mipmaps = mipmaps;
        // Already enough allocated
        return IMGLOAD_ERR_NO_ERROR;
    }

    Mipmap* new_data = (Mipmap*)mem_reallocz(img->context, NULL, mipmaps * sizeof(*frame->mipmaps));
    if (new_data == NULL)
    {
        return IMGLOAD_ERR_OUT_OF_MEMORY;
    }

    frame->n_mipmaps = mipmaps;
    frame->mipmaps = new_data;

    return IMGLOAD_ERR_NO_ERROR;
}

void image_set_compressed_data(ImgloadImage img, size_t subframe, size_t mipmap,
                                            ImgloadImageData* data, bool transfer_ownership)
{
    Mipmap* mipmap1 = &img->frames[subframe].mipmaps[mipmap];
    mipmap1->compressed.image = *data;

    if (!transfer_ownership)
    {
        // Memory wasn't allocated by us so we need to copy it.
        mipmap1->compressed.image.data = mem_realloc(img->context, NULL, data->data_size);

        if (mipmap1->compressed.image.data == NULL)
        {
            print_to_log(img->context, IMGLOAD_LOG_ERROR, "Failed to allocate memory for copying image data from plugin!");
            return;
        }

        memcpy(mipmap1->compressed.image.data, data->data, data->data_size);
    }

    mipmap1->compressed.has_data = true;
}

void image_set_data(ImgloadImage img, size_t subframe, size_t mipmap,
                                            ImgloadImageData* data, bool transfer_ownership)
{
    Mipmap* mipmap1 = &img->frames[subframe].mipmaps[mipmap];
    mipmap1->raw.image = *data;

    if (!transfer_ownership)
    {
        // Memory wasn't allocated by us so we need to copy it.
        mipmap1->raw.image.data = mem_realloc(img->context, NULL, data->data_size);

        if (mipmap1->raw.image.data == NULL)
        {
            print_to_log(img->context, IMGLOAD_LOG_ERROR, "Failed to allocate memory for copying image data from plugin!");
            return;
        }

        memcpy(mipmap1->raw.image.data, data->data, data->data_size);
    }
    mipmap1->raw.has_data = true;

    if (img->context->flags & IMGLOAD_CONTEXT_FLIP_IMAGES)
    {
        // A buffer for holding one line of the image
        uint8_t* buffer = (uint8_t*) mem_realloc(img->context, NULL, data->stride);

        if (buffer == NULL)
        {
            print_to_log(img->context, IMGLOAD_LOG_ERROR, "Failed to allocate buffer for flipping image!");
            return;
        }

        for (size_t d = 0; d < data->depth; ++d)
        {
            uint8_t* image_data = ((uint8_t*)data->data) + (d * data->height * data->stride);

            // Images that have a height that's not divisible by 2 don't need to be handled
            // specially because the row in the middle of the image doesn't need to be mirrored
            size_t height_half = data->height / 2;
            for (size_t y = 0; y < height_half; ++y)
            {
                uint8_t* top = image_data + y * data->stride;
                uint8_t* bottom = image_data + (data->height - y - 1) * data->stride;

                memcpy(buffer, top, data->stride);
                memcpy(top, bottom, data->stride);
                memcpy(bottom, buffer, data->stride);
            }
        }

        mem_free(img->context, buffer);
    }
}


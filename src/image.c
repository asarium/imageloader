#include <string.h>
#include <assert.h>
#include <stdio.h>

#include <ddsimg/ddsimg.h>

#include "image.h"
#include "memory.h"
#include "dxtc.h"
#include "util.h"

static DDSCompressedFormat_t get_format(DDS_HEADER* header)
{
    if (header->ddspf.dwFlags & DDS_FOURCC)
    {
        switch (header->ddspf.dwFourCC)
        {
            case DDSIMG_MAKEFOURCC('D', 'X', 'T', '1'):
                return DDSIMG_COM_DXT1;

            case DDSIMG_MAKEFOURCC('D', 'X', 'T', '2'):
                return DDSIMG_COM_DXT2;

            case DDSIMG_MAKEFOURCC('D', 'X', 'T', '3'):
                return DDSIMG_COM_DXT3;

            case DDSIMG_MAKEFOURCC('D', 'X', 'T', '4'):
                return DDSIMG_COM_DXT4;

            case DDSIMG_MAKEFOURCC('D', 'X', 'T', '5'):
                return DDSIMG_COM_DXT5;

            default:
                return DDSIMG_COM_UNKNOWN;
        }
    }

    return DDSIMG_COM_UNKNOWN;
}

static DDSErrorCode check_header(DDS_HEADER* header)
{
    if (header->dwSize != 124 && header->dwSize != DDSIMG_MAKEFOURCC('D', 'D', 'S', ' '))
        return DDSIMG_ERR_FILE_INVALID;
    if (header->ddspf.dwSize != 32)
        return DDSIMG_ERR_FILE_INVALID;
    if (header->dwWidth == 0 || header->dwHeight == 0)
        return DDSIMG_ERR_FILE_INVALID;

    return DDSIMG_ERR_NO_ERROR;
}

static void fix_header(DDSImage* image)
{
    if (!(image->header.dwFlags & DDS_MIPMAPCOUNT) || image->header.dwMipMapCount == 0)
    {
        image->header.dwMipMapCount = 1;
    }
}

static int has_cubemap(DDSImage* image, uint32_t index)
{
    switch (index)
    {
        case 0:
            return (image->header.dwCaps2 & DDS2_CUBEMAP_POSITIVEX) != 0;
        case 1:
            return (image->header.dwCaps2 & DDS2_CUBEMAP_NEGATIVEX) != 0;
        case 2:
            return (image->header.dwCaps2 & DDS2_CUBEMAP_POSITIVEY) != 0;
        case 3:
            return (image->header.dwCaps2 & DDS2_CUBEMAP_NEGATIVEY) != 0;
        case 4:
            return (image->header.dwCaps2 & DDS2_CUBEMAP_POSITIVEZ) != 0;
        case 5:
            return (image->header.dwCaps2 & DDS2_CUBEMAP_NEGATIVEZ) != 0;
        default:
            return 0;
    }
}

static DDSErrorCode fill_compressed_data_pointers(DDSImage* image, subimage_t* subimage)
{
    subimage->compressed = ddsimg_realloc(image->ctx, NULL, sizeof(MipmapData) * image->header.dwMipMapCount);
    if (!subimage->compressed)
    {
        return DDSIMG_ERR_OUT_OF_MEMORY;
    }

    memset(subimage->compressed, 0, sizeof(MipmapData) * image->header.dwMipMapCount);

    uint32_t width = image->header.dwWidth;
    uint32_t height = image->header.dwHeight;
    uint32_t depth = image->header.dwDepth;

    size_t i;
    for (i = 0; i < image->header.dwMipMapCount; ++i)
    {
        MipmapData* mipmap = &subimage->compressed[i];

        size_t mipmap_size = dxtc_get_data_size(image->props.compression_format, width, height) * depth;

        mipmap->data_size = mipmap_size;
        mipmap->width = width;
        mipmap->height = height;
        mipmap->depth = depth;

        width = MAX(1, width >> 1);
        height = MAX(1, height >> 1);
        depth = MAX(1, depth >> 1);
    }

    return DDSIMG_ERR_NO_ERROR;
}

static DDSErrorCode fill_subimages(DDSImage* image)
{
    image->subimages = ddsimg_realloc(image->ctx, NULL, sizeof(subimage_t) * image->props.subimages);
    if (image->subimages == NULL)
    {
        return DDSIMG_ERR_OUT_OF_MEMORY;
    }
    memset(image->subimages, 0, sizeof(subimage_t) * image->props.subimages);

    uint32_t i;
    for (i = 0; i < image->props.subimages; ++i)
    {
        DDSErrorCode err = fill_compressed_data_pointers(image, &image->subimages[i]);

        if (err != DDSIMG_ERR_NO_ERROR)
        {
            return err;
        }
    }

    return DDSIMG_ERR_NO_ERROR;
}

DDSErrorCode DDSIMG_API ddsimg_image_alloc(DDSContext* context, DDSImage** image, DDSIOFunctions* io_funcs, void* io_ud)
{
    if (context == NULL)
    {
        return DDSIMG_ERR_INVALID_ARGUMENT;
    }
    if (image == NULL)
    {
        return DDSIMG_ERR_INVALID_ARGUMENT;
    }
    if (io_funcs == NULL)
    {
        return DDSIMG_ERR_INVALID_ARGUMENT;
    }

    DDSImage* img = ddsimg_realloc(context, NULL, sizeof(DDSImage));
    if (img == NULL)
    {
        return DDSIMG_ERR_OUT_OF_MEMORY;
    }
    memset(img, 0, sizeof(DDSImage));

    *image = img;

    img->ctx = context;
    img->io.funcs = *io_funcs;
    img->io.ud = io_ud;

    return DDSIMG_ERR_NO_ERROR;
}

DDSErrorCode DDSIMG_API ddsimg_image_read_header(DDSImage* image)
{
    if (image == NULL)
    {
        return DDSIMG_ERR_INVALID_ARGUMENT;
    }

    ddsimg_seek(image, 0, SEEK_SET);

    uint32_t magic;
    if (ddsimg_read(image, (uint8_t*) &magic, sizeof(magic)) != sizeof(magic))
    {
        // No magic value
        return DDSIMG_ERR_FILE_INVALID;
    }

    // Check for the 'DDS ' magic value
    if (magic != 0x20534444)
    {
        return DDSIMG_ERR_FILE_INVALID;
    }

    if (ddsimg_read(image, (uint8_t*) &image->header, sizeof(image->header)) != sizeof(image->header))
    {
        // Not enough data for the header
        return DDSIMG_ERR_FILE_INVALID;
    }

    if (image->header.dwDepth == 0)
    {
        image->header.dwDepth = 1;
    }

    // Header has been successfully been read into the image, no check if the values are valid
    DDSErrorCode check_code = check_header(&image->header);

    if (check_code != DDSIMG_ERR_NO_ERROR)
    {
        // Something is invalid
        return check_code;
    }

    image->props.compression_format = get_format(&image->header);

    if (image->props.compression_format == DDSIMG_COM_UNKNOWN)
    {
        return DDSIMG_ERR_FORMAT_UNKNOWN;
    }

    // We currently always decompress to 32 bit RGBA
    // This needs to be changed should uncompressed input be supported at some point
    image->props.decompressed_format = DDSIMG_FORMAT_R8G8B8A8;

    fix_header(image);

    // By default a dds file contains one image (+ mipmaps)
    image->props.subimages = 1;
    if (image->header.dwCaps & DDS_COMPLEX)
    {
        if (image->header.dwCaps & DDS2_CUBEMAP)
        {
            image->props.subimages = 6; // Always use 6 subimages for cubemaps
            image->props.flags |= DDSIMG_IMG_CUBEMAP;
        }
    }

    DDSErrorCode fill_error = fill_subimages(image);

    if (fill_error != DDSIMG_ERR_NO_ERROR)
    {
        return fill_error;
    }

    size_t total_data_size = 0;
    size_t i;
    uint32_t subimage;
    for (subimage = 0; subimage < image->props.subimages; ++subimage)
    {
        if (image->props.flags & DDSIMG_IMG_CUBEMAP && !has_cubemap(image, subimage))
        {
            continue;
        }

        for (i = 0; i < image->header.dwMipMapCount; ++i)
        {
            total_data_size += image->subimages[subimage].compressed[i].data_size;
        }
    }

    // The expected size of the file
    size_t expected_size = 4 + sizeof(DDS_HEADER) + total_data_size;

    // Seek to the end of the file to check if the size matches
    int64_t end_off = ddsimg_seek(image, 0, SEEK_END);

    if (end_off < (int64_t) expected_size)
    {
        // Not enough data in file
        return DDSIMG_ERR_FILE_SIZE_WRONG;
    }

    image->header_parsed = 1;
    return DDSIMG_ERR_NO_ERROR;
}

DDSErrorCode DDSIMG_API ddsimg_image_get_size(DDSImage* image, uint32_t* width_out, uint32_t* height_out,
                                              uint32_t* depth_out)
{
    if (image == NULL)
    {
        return DDSIMG_ERR_INVALID_ARGUMENT;
    }

    if (!image->header_parsed)
    {
        // This call is only valid if the header has been read
        return DDSIMG_ERR_INVALID_API_USAGE;
    }

    if (width_out)
    {
        *width_out = image->header.dwWidth;
    }

    if (height_out)
    {
        *height_out = image->header.dwHeight;
    }

    if (depth_out)
    {
        *depth_out = image->header.dwDepth;
    }

    return DDSIMG_ERR_NO_ERROR;
}

DDSErrorCode DDSIMG_API ddsimg_image_get_flags(DDSImage* image, uint32_t* flags_out)
{
    if (image == NULL)
    {
        return DDSIMG_ERR_INVALID_ARGUMENT;
    }

    if (!image->header_parsed)
    {
        // This call is only valid if the header has been read
        return DDSIMG_ERR_INVALID_API_USAGE;
    }

    if (flags_out)
    {
        *flags_out = image->props.flags;
    }

    return DDSIMG_ERR_NO_ERROR;
}


DDSErrorCode DDSIMG_API ddsimg_image_get_compression(DDSImage* image, uint32_t* comp_out)
{
    if (image == NULL)
    {
        return DDSIMG_ERR_INVALID_ARGUMENT;
    }

    if (!image->header_parsed)
    {
        // This call is only valid if the header has been read
        return DDSIMG_ERR_INVALID_API_USAGE;
    }

    if (comp_out)
    {
        *comp_out = (uint32_t) image->props.compression_format;
    }

    return DDSIMG_ERR_NO_ERROR;
}

DDSErrorCode DDSIMG_API ddsimg_image_get_format(DDSImage* image, uint32_t* format_out)
{
    if (image == NULL)
    {
        return DDSIMG_ERR_INVALID_ARGUMENT;
    }

    if (!image->header_parsed)
    {
        // This call is only valid if the header has been read
        return DDSIMG_ERR_INVALID_API_USAGE;
    }

    if (format_out)
    {
        *format_out = (uint32_t) image->props.decompressed_format;
    }

    return DDSIMG_ERR_NO_ERROR;
}

DDSErrorCode DDSIMG_API ddsimg_image_get_num_mipmaps(DDSImage* image, uint32_t* num_mipmaps_out)
{
    if (image == NULL)
    {
        return DDSIMG_ERR_INVALID_ARGUMENT;
    }

    if (!image->header_parsed)
    {
        // This call is only valid if the header has been read
        return DDSIMG_ERR_INVALID_API_USAGE;
    }

    if (num_mipmaps_out)
    {
        *num_mipmaps_out = image->header.dwMipMapCount;
    }

    return DDSIMG_ERR_NO_ERROR;
}

DDSErrorCode DDSIMG_API ddsimg_image_get_num_subimages(DDSImage* image, uint32_t* num_subimages)
{
    if (image == NULL)
    {
        return DDSIMG_ERR_INVALID_ARGUMENT;
    }

    if (!image->header_parsed)
    {
        // This call is only valid if the header has been read
        return DDSIMG_ERR_INVALID_API_USAGE;
    }

    if (num_subimages)
    {
        *num_subimages = image->props.subimages;
    }

    return DDSIMG_ERR_NO_ERROR;
}

DDSErrorCode DDSIMG_API ddsimg_image_read_data(DDSImage* image)
{
    if (image == NULL)
    {
        return DDSIMG_ERR_INVALID_ARGUMENT;
    }

    if (!image->header_parsed)
    {
        // This call is only valid if the header has been read
        return DDSIMG_ERR_INVALID_API_USAGE;
    }

    ddsimg_seek(image, 4 + sizeof(DDS_HEADER), SEEK_SET);

    uint32_t subimage;
    uint32_t mipmap_index;

    for (subimage = 0; subimage < image->props.subimages; ++subimage)
    {
        if (image->props.flags & DDSIMG_IMG_CUBEMAP)
        {
            if (!has_cubemap(image, subimage))
            {
                // If the cubemap is missing from the file then just read the next face
                continue;
            }
        }

        for (mipmap_index = 0; mipmap_index < image->header.dwMipMapCount; ++mipmap_index)
        {
            MipmapData* mipmap = &image->subimages[subimage].compressed[mipmap_index];

            mipmap->data = ddsimg_realloc(image->ctx, NULL, mipmap->data_size);
            if (mipmap->data == NULL)
            {
                return DDSIMG_ERR_OUT_OF_MEMORY;
            }

            size_t actual_read = ddsimg_read(image, (uint8_t*) mipmap->data, mipmap->data_size);

            if (actual_read != mipmap->data_size)
            {
                return DDSIMG_ERR_FILE_SIZE_WRONG;
            }
        }
    }

    image->data_read = 1;

    return DDSIMG_ERR_NO_ERROR;
}

DDSErrorCode DDSIMG_API ddsimg_image_get_compressed_data(DDSImage* image, uint32_t subimage, uint32_t mipmap_index,
                                                         MipmapData* data)
{
    if (!image )
    {
        return DDSIMG_ERR_INVALID_ARGUMENT;
    }
    if (!image->data_read)
    {
        return DDSIMG_ERR_INVALID_API_USAGE;
    }
    if (!data)
    {
        return DDSIMG_ERR_INVALID_ARGUMENT;
    }

    if (subimage >= image->props.subimages)
    {
        return DDSIMG_ERR_INVALID_ARGUMENT;
    }
    if (mipmap_index >= image->header.dwMipMapCount)
    {
        return DDSIMG_ERR_INVALID_ARGUMENT;
    }

    if (image->subimages[subimage].compressed->data == NULL)
    {
        // This face is not available
        return DDSIMG_ERR_NO_DATA;
    }

    *data = image->subimages[subimage].compressed[mipmap_index];

    return DDSIMG_ERR_NO_ERROR;
}

DDSErrorCode DDSIMG_API ddsimg_image_get_decompressed_data(DDSImage* image, uint32_t subimage_index,
                                                           uint32_t mipmap_index,
                                                           MipmapData* data)
{
    if (image == NULL)
    {
        return DDSIMG_ERR_INVALID_ARGUMENT;
    }
    if (!image->data_read)
    {
        return DDSIMG_ERR_INVALID_API_USAGE;
    }
    if (!data)
    {
        return DDSIMG_ERR_INVALID_ARGUMENT;
    }

    if (subimage_index >= image->props.subimages)
    {
        return DDSIMG_ERR_INVALID_ARGUMENT;
    }
    if (mipmap_index >= image->header.dwMipMapCount)
    {
        return DDSIMG_ERR_INVALID_ARGUMENT;
    }

    if (image->subimages[subimage_index].compressed->data == NULL)
    {
        // This face is not available
        return DDSIMG_ERR_NO_DATA;
    }

    subimage_t* subimage = &image->subimages[subimage_index];
    // Allocate the data structure for decompressed mipmaps
    if (subimage->decompressed == NULL)
    {
        subimage->decompressed = ddsimg_realloc(image->ctx, NULL, sizeof(MipmapData) * image->header.dwMipMapCount);
        if (!subimage->decompressed)
        {
            return DDSIMG_ERR_OUT_OF_MEMORY;
        }
        memset(subimage->decompressed, 0, sizeof(MipmapData) * image->header.dwMipMapCount);

        // Copy size from compressed data
        uint32_t i;
        for (i = 0; i < image->header.dwMipMapCount; ++i)
        {
            subimage->decompressed->width = subimage->compressed->width;
            subimage->decompressed->height = subimage->compressed->height;
            subimage->decompressed->depth = subimage->compressed->depth;
        }
    }

    MipmapData* decompressed_mipmap = &subimage->decompressed[mipmap_index];

    if (decompressed_mipmap->data == NULL)
    {
        // Need to decompress first
        DDSErrorCode err = dxtc_decrompress(image, &subimage->compressed[mipmap_index], decompressed_mipmap);

        if (err != DDSIMG_ERR_NO_ERROR)
        {
            // Free data if something bad happened
            ddsimg_free(image->ctx, decompressed_mipmap->data);
            decompressed_mipmap->data = NULL;

            // Something went wrong
            return err;
        }
    }

    *data = *decompressed_mipmap;

    return DDSIMG_ERR_NO_ERROR;
}

static void free_data_pointers(DDSContext* ctx, MipmapData** ptrs, uint32_t num_mipmaps)
{
    if (*ptrs == NULL)
    {
        // Nothing allocated
        return;
    }
    MipmapData* mipmap_array = *ptrs;

    uint32_t mipmap;
    for (mipmap = 0; mipmap < num_mipmaps; ++mipmap)
    {
        MipmapData* mipmap_ptr = &mipmap_array[mipmap];

        if (mipmap_ptr->data == NULL)
        {
            continue;
        }

        ddsimg_free(ctx, mipmap_ptr->data);
    }
    ddsimg_free(ctx, mipmap_array);

    // Reset value so no one accidentally accessed freed data
    *ptrs = NULL;
}

DDSErrorCode DDSIMG_API ddsimg_image_free(DDSImage** image_ptr)
{
    if (image_ptr == NULL)
    {
        return DDSIMG_ERR_INVALID_ARGUMENT;
    }
    if (*image_ptr == NULL)
    {
        return DDSIMG_ERR_INVALID_ARGUMENT;
    }

    DDSImage* img = *image_ptr;

    uint32_t i;
    for (i = 0; i < img->props.subimages; ++i)
    {
        free_data_pointers(img->ctx, &img->subimages[i].compressed, img->header.dwMipMapCount);
        free_data_pointers(img->ctx, &img->subimages[i].decompressed, img->header.dwMipMapCount);
    }
    ddsimg_free(img->ctx, img->subimages);

    ddsimg_free(img->ctx, img);
    *image_ptr = NULL;

    return DDSIMG_ERR_NO_ERROR;
}

size_t ddsimg_read(DDSImage* img, uint8_t* buf, size_t buf_len)
{
    assert(img != NULL);

    return img->io.funcs.read(img->io.ud, buf, buf_len);
}

int64_t ddsimg_seek(DDSImage* img, int64_t offset, int whence)
{
    assert(img != NULL);

    return img->io.funcs.seek(img->io.ud, offset, whence);
}

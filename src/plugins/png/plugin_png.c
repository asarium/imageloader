
#include "plugin_png.h"

#include <imageloader_plugin.h>

#include <png.h>

#include <stdbool.h>
#include <inttypes.h>

// Parts of this code are based on this tutorial: http://www.piko3d.net/tutorials/libpng-tutorial-loading-png-files-from-streams/

typedef struct
{
    png_structp png_ptr;
    png_infop info_ptr;
} PNGPointers;

#define png_error_occured(png_ptr) setjmp(png_jmpbuf(png_ptr)) != 0

static png_voidp png_malloc_fn(png_structp png_ptr, png_size_t size)
{
    ImgloadPlugin plugin = (ImgloadPlugin)png_get_mem_ptr(png_ptr);

    return imgload_plugin_realloc(plugin, NULL, size);
}

static void png_free_fn(png_structp png_ptr, png_voidp ptr)
{
    ImgloadPlugin plugin = (ImgloadPlugin)png_get_mem_ptr(png_ptr);

    imgload_plugin_free(plugin, ptr);
}


static void png_user_read_data(png_structp png_ptr, png_bytep data, png_size_t length)
{
    ImgloadImage img = (ImgloadImage)png_get_io_ptr(png_ptr);

    size_t read = imgload_plugin_image_read(img, (uint8_t*)data, length);

    if (read != length)
    {
        png_error(png_ptr, "Read Error");
    }
}


static void png_error_fn(png_structp png_ptr, png_const_charp message)
{
    ImgloadPlugin plugin = (ImgloadPlugin)png_get_error_ptr(png_ptr);

    imgload_plugin_log(plugin, IMGLOAD_LOG_ERROR, message);
    
    longjmp(png_jmpbuf(png_ptr), 1);
}

static void png_warning_fn(png_structp png_ptr, png_const_charp message)
{
    ImgloadPlugin plugin = (ImgloadPlugin)png_get_error_ptr(png_ptr);

    imgload_plugin_log(plugin, IMGLOAD_LOG_WARNING, message);
}


#define PNGSIGSIZE 8
static int IMGLOAD_CALLBACK png_probe(ImgloadPlugin plugin, ImgloadImage img)
{
    png_byte pngsig[PNGSIGSIZE];

    if (imgload_plugin_image_read(img, pngsig, PNGSIGSIZE) != PNGSIGSIZE)
    {
        return 0;
    }

    return png_sig_cmp(pngsig, 0, PNGSIGSIZE) == 0;
}

static ImgloadErrorCode IMGLOAD_CALLBACK png_init_image(ImgloadPlugin plugin, ImgloadImage img)
{
    png_structp png_ptr = png_create_read_struct_2(PNG_LIBPNG_VER_STRING, plugin, png_error_fn, png_warning_fn, plugin, png_malloc_fn, png_free_fn);
    if (png_ptr == NULL)
    {
        return IMGLOAD_ERR_PLUGIN_ERROR;
    }

    png_infop png_info = png_create_info_struct(png_ptr);
    if (png_info == NULL)
    {
        png_destroy_read_struct(&png_ptr, NULL, NULL);
        return IMGLOAD_ERR_PLUGIN_ERROR;
    }

    if (png_error_occured(png_ptr))
    {
        // libPNG has caused an error, free memory and return
        png_destroy_read_struct(&png_ptr, &png_info, NULL);
        return IMGLOAD_ERR_PLUGIN_ERROR;
    }

    png_set_read_fn(png_ptr, img, png_user_read_data);

    png_set_sig_bytes(png_ptr, PNGSIGSIZE);

    png_read_info(png_ptr, png_info);

    // Info has been read
    png_uint_32 img_width = png_get_image_width(png_ptr, png_info);
    png_uint_32 img_height = png_get_image_height(png_ptr, png_info);

    png_uint_32 bitdepth = png_get_bit_depth(png_ptr, png_info);
    png_uint_32 channels = png_get_channels(png_ptr, png_info);
    png_uint_32 color_type = png_get_color_type(png_ptr, png_info);

    if (bitdepth == 16)
    {
        png_set_strip_16(png_ptr);
    }

    switch (color_type) {
    case PNG_COLOR_TYPE_PALETTE:
        // Expand palette to rgb
        png_set_palette_to_rgb(png_ptr);
        break;
    case PNG_COLOR_TYPE_GRAY:
        if (bitdepth < 8)
            png_set_expand_gray_1_2_4_to_8(png_ptr);
        break;
    }

    // if the image has a transperancy set.. convert it to a full Alpha channel..
    // Also make sure that is was RGB before
    if (png_get_valid(png_ptr, png_info, PNG_INFO_tRNS))
    {
        png_set_tRNS_to_alpha(png_ptr);
    }

    // Update the structure so we can use it later
    png_read_update_info(png_ptr, png_info);

    bitdepth = png_get_bit_depth(png_ptr, png_info);
    channels = png_get_channels(png_ptr, png_info);
    color_type = png_get_color_type(png_ptr, png_info);

    if (bitdepth != 8)
    {
        // Any bitdepth != 8 is not supported
        png_destroy_read_struct(&png_ptr, &png_info, NULL);

        imgload_plugin_log(plugin, IMGLOAD_LOG_ERROR, "PNG has a bitdepth of %"PRIu32" but only a bitdepth of 8 is supported by this plugin!", bitdepth);

        return IMGLOAD_ERR_UNSUPPORTED_FORMAT;
    }

    // Convert PNG format to imageloader. Also validates channel number
    ImgloadFormat format;
    if (color_type == PNG_COLOR_TYPE_RGB && channels == 3)
    {
        format = IMGLOAD_FORMAT_R8G8B8;
    } else if (color_type == PNG_COLOR_TYPE_RGBA && channels == 4)
    {
        format = IMGLOAD_FORMAT_R8G8B8A8;
    } else if (color_type == PNG_COLOR_TYPE_GRAY && channels == 1)
    {
        format = IMGLOAD_FORMAT_GRAY8;
    } else
    {
        // Currently no other format is supported
        png_destroy_read_struct(&png_ptr, &png_info, NULL);

        imgload_plugin_log(plugin, IMGLOAD_LOG_ERROR, "PNG has an unsupported data format!");

        return IMGLOAD_ERR_UNSUPPORTED_FORMAT;
    }

    // Everything seems to be alright, set imageloader properties
    imgload_plugin_image_set_data_type(img, format, IMGLOAD_COMPRESSION_NONE);
    imgload_plugin_image_set_num_frames(img, 1);
    imgload_plugin_image_set_num_mipmaps(img, 0, 1);

    imgload_plugin_image_set_property(img, 0, IMGLOAD_PROPERTY_WIDTH, IMGLOAD_PROPERTY_TYPE_UINT32, &img_width);
    imgload_plugin_image_set_property(img, 0, IMGLOAD_PROPERTY_HEIGHT, IMGLOAD_PROPERTY_TYPE_UINT32, &img_height);

    // PNGs are always 2D so set depth to 1
    uint32_t one = 1;
    imgload_plugin_image_set_property(img, 0, IMGLOAD_PROPERTY_DEPTH, IMGLOAD_PROPERTY_TYPE_UINT32, &one);

    PNGPointers* pointers = (PNGPointers*)imgload_plugin_realloc(plugin, NULL, sizeof(PNGPointers));
    if (pointers == NULL)
    {
        // Currently no other format is supported
        png_destroy_read_struct(&png_ptr, &png_info, NULL);
        return IMGLOAD_ERR_OUT_OF_MEMORY;
    }

    pointers->png_ptr = png_ptr;
    pointers->info_ptr = png_info;

    imgload_plugin_image_set_data(img, pointers);
    return IMGLOAD_ERR_NO_ERROR;
}

static ImgloadErrorCode IMGLOAD_CALLBACK png_read_data(ImgloadPlugin plugin, ImgloadImage img)
{
    PNGPointers* pointers = (PNGPointers*)imgload_plugin_image_get_data(img);

    png_structp png_ptr = pointers->png_ptr;
    png_infop png_info = pointers->info_ptr;

    png_uint_32 img_width = png_get_image_width(png_ptr, png_info);
    png_uint_32 img_height = png_get_image_height(png_ptr, png_info);

    //Here's one of the pointers we've defined in the error handler section:
    //Array of row pointers. One for every row.
    png_bytepp rowPtrs = (png_bytepp)imgload_plugin_realloc(plugin, NULL, img_height * sizeof(png_bytep));
    if (rowPtrs == NULL)
    {
        return IMGLOAD_ERR_OUT_OF_MEMORY;
    }

    //This is the length in bytes, of one row.
    size_t stride = png_get_rowbytes(png_ptr, png_info);
    size_t total_size = img_height * stride;

    //Allocate a buffer with enough space.
    png_byte* data = (png_byte*)imgload_plugin_realloc(plugin, NULL, total_size);
    if (data == NULL)
    {
        imgload_plugin_free(plugin, rowPtrs);
        return IMGLOAD_ERR_OUT_OF_MEMORY;
    }

    //A little for-loop here to set all the row pointers to the starting
    //Adresses for every row in the buffer

    for (size_t i = 0; i < img_height; i++) {
        size_t q = i * stride;
        rowPtrs[i] = data + q;
    }

    if (png_error_occured(png_ptr))
    {
        // Something went wrong, PANIC!!!
        imgload_plugin_free(plugin, rowPtrs);
        imgload_plugin_free(plugin, data);

        return IMGLOAD_ERR_PLUGIN_ERROR;
    }

    //And here it is! The actuall reading of the image!
    //Read the imagedata and write it to the adresses pointed to
    //by rowptrs (in other words: our image databuffer)
    png_read_image(png_ptr, rowPtrs);

    // Everything should be fine here, now set the data and go home
    ImgloadImageData img_data;
    img_data.width = img_width;
    img_data.height = img_height;
    img_data.depth = 1;

    img_data.data_size = total_size;
    img_data.data = data;

    // The memory was allocated using the imageloader allocator so we can transfer ownership
    imgload_plugin_image_set_image_data(img, 0, 0, &img_data, 1);

    // The row pointers aren't needed anymore
    imgload_plugin_free(plugin, rowPtrs);

    return IMGLOAD_ERR_NO_ERROR;
}

static ImgloadErrorCode IMGLOAD_CALLBACK png_deinit_image(ImgloadPlugin plugin, ImgloadImage img)
{
    PNGPointers* pointers = (PNGPointers*)imgload_plugin_image_get_data(img);

    png_destroy_read_struct(&pointers->png_ptr, &pointers->info_ptr, NULL);
    imgload_plugin_free(plugin, pointers);

    return IMGLOAD_ERR_NO_ERROR;
}

ImgloadErrorCode IMGLOAD_CALLBACK png_plugin_loader(ImgloadPlugin plugin, void* parameter)
{
    imgload_plugin_set_info(plugin, "png", "libPNG plugin", "Loads PNG files using libpng");

    imgload_plugin_callback_probe(plugin, png_probe);

    imgload_plugin_callback_init_image(plugin, png_init_image);
    imgload_plugin_callback_deinit_image(plugin, png_deinit_image);

    imgload_plugin_callback_read_data(plugin, png_read_data);

    return IMGLOAD_ERR_NO_ERROR;
}
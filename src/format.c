//
//

#include "format.h"
#include "memory.h"
#include "packed.h"

#include <assert.h>

typedef PACK(struct
                     {
                         uint8_t r;
                         uint8_t g;
                         uint8_t b;
                         uint8_t a;
                     }) color_rgba;

typedef PACK(struct
                     {
                         uint8_t r;
                         uint8_t g;
                         uint8_t b;
                     }) color_rgb;

size_t format_bpp(ImgloadFormat format)
{
    switch (format)
    {
        case IMGLOAD_FORMAT_R8G8B8A8:
            return 4;
        case IMGLOAD_FORMAT_B8G8R8A8:
            return 4;
        case IMGLOAD_FORMAT_R8G8B8:
            return 3;
        case IMGLOAD_FORMAT_GRAY8:
            return 1;
        default:
            return 0;
    }
}

static void convert_rgba_bgra(ImgloadImageData* img_data)
{

    size_t slice_size = img_data->height * img_data->stride;

    uint8_t* data_begin = (uint8_t*) img_data->data;

    for (size_t d = 0; d < img_data->depth; ++d)
    {
        for (size_t y = 0; y < img_data->height; ++y)
        {
            uint8_t* line_begin = data_begin + d * slice_size + y * img_data->stride;

            color_rgba* colors = (color_rgba*) line_begin;

            for (size_t x = 0; x < img_data->width; ++x)
            {
                color_rgba c = colors[x];

                color_rgba* dest_color = colors + x;

                // Switch r and b, the rest stays the same
                dest_color->r = c.b;
                dest_color->b = c.r;
            }
        }
    }
}

typedef void (* ConverterFunction)(ImgloadImage img, void* input_ptr, void* output_ptr);

static void convert_generic(ImgloadImage img, ImgloadImageData* input, ImgloadFormat input_fmt,
                            ImgloadImageData* output,
                            ImgloadFormat output_fmt, ConverterFunction converter)
{
    size_t input_slice_size = input->height * input->stride;
    size_t output_slice_size = output->height * output->stride;

    uint8_t* input_begin = (uint8_t*) input->data;
    uint8_t* output_begin = (uint8_t*) output->data;

    size_t input_bpp = format_bpp(input_fmt);
    size_t output_bpp = format_bpp(output_fmt);

    for (size_t d = 0; d < input->depth; ++d)
    {
        for (size_t y = 0; y < input->height; ++y)
        {
            uint8_t* input_line_begin = input_begin + d * input_slice_size + y * input->stride;
            uint8_t* output_line_begin = output_begin + d * output_slice_size + y * output->stride;

            for (size_t x = 0; x < input->width; ++x)
            {
                uint8_t* input_ptr = input_line_begin + x * input_bpp;
                uint8_t* output_ptr = output_line_begin + x * output_bpp;

                converter(img, input_ptr, output_ptr);
            }
        }
    }
}

static void rgb_to_rgba(ImgloadImage img, void* input_ptr, void* output_ptr)
{
    color_rgb* input_color = (color_rgb*) input_ptr;
    color_rgba* output_color = (color_rgba*) output_ptr;

    output_color->r = input_color->r;
    output_color->g = input_color->g;
    output_color->b = input_color->b;
    output_color->a = (uint8_t) img->conv.param; // The parameter is the alpha value
}

static void rgba_to_rgb(ImgloadImage img, void* input_ptr, void* output_ptr)
{
    color_rgba* input_color = (color_rgba*) input_ptr;
    color_rgb* output_color = (color_rgb*) output_ptr;

    output_color->r = input_color->r;
    output_color->g = input_color->g;
    output_color->b = input_color->b;
}

static void rgb_to_luminance(ImgloadImage img, void* input_ptr, void* output_ptr)
{
    float r_factor = 0.2126;
    float g_factor = 0.7152;
    float b_factor = 0.0722;

    if (img->conv.param != 0)
    {
        uint64_t param = img->conv.param;
        // Use the lowest 3 bytes to determine the factors
        uint64_t r_ratio = (param & 0xFF0000) >> 16;
        uint64_t g_ratio = (param & 0x00FF00) >> 8;
        uint64_t b_ratio = (param & 0x0000FF);

        uint64_t sum = r_ratio + g_ratio + b_ratio;

        r_factor = (float)r_ratio / sum;
        g_factor = (float)g_ratio / sum;
        b_factor = (float)b_ratio / sum;
    }

    color_rgb* input_color = (color_rgb*) input_ptr;
    uint8_t* output_color = (uint8_t*) output_ptr;

    *output_color = (uint8_t) (input_color->r * r_factor + input_color->g * g_factor + input_color->b * b_factor);
}

static void luminance_to_rgba(ImgloadImage img, void* input_ptr, void* output_ptr)
{
    uint8_t* input_color = (uint8_t*) input_ptr;
    color_rgba* output_color = (color_rgba*) output_ptr;

    output_color->r = *input_color;
    output_color->g = *input_color;
    output_color->b = *input_color;
    output_color->a = (uint8_t) img->conv.param; // The parameter is the alpha value
}

static void luminance_to_rgb(ImgloadImage img, void* input_ptr, void* output_ptr)
{
    uint8_t* input_color = (uint8_t*) input_ptr;
    color_rgb* output_color = (color_rgb*) output_ptr;

    output_color->r = *input_color;
    output_color->g = *input_color;
    output_color->b = *input_color;
}

ImgloadErrorCode format_change(ImgloadImage img, ImgloadFormat current, ImgloadImageData* data,
                               ImgloadImageData* converted_out)
{
    assert(img != NULL);
    assert(converted_out != NULL);
    assert(data != NULL);
    assert(data->data != NULL);

    ImgloadFormat destination = img->conv.requested;

    if (current == destination)
    {
        // No conversion needed
        *converted_out = *data;
        return IMGLOAD_ERR_NO_ERROR;
    }

    // When both formats use the same amount of memory then the conversion can happen in-place
    bool in_place = format_bpp(current) == format_bpp(destination);

    converted_out->depth = data->depth;
    converted_out->width = data->width;
    converted_out->height = data->height;

    if (in_place)
    {
        converted_out->stride = data->stride;
        converted_out->data_size = data->data_size;
        converted_out->data = data->data;
    }
    else
    {
        size_t converted_stride = data->width * format_bpp(destination);
        size_t converted_size = data->depth * data->height * converted_stride;
        uint8_t* converted_data = mem_realloc(img->context, NULL, converted_size);

        if (converted_data == NULL)
        {
            return IMGLOAD_ERR_OUT_OF_MEMORY;
        }

        converted_out->stride = converted_stride;
        converted_out->data_size = converted_size;
        converted_out->data = converted_data;
    }

    ImgloadErrorCode err = IMGLOAD_ERR_NO_ERROR;

    // A giant switch-case for handling all possible combinations, if you have a better solution, let me know...
    switch(current)
    {
        case IMGLOAD_FORMAT_R8G8B8A8:
            switch(destination)
            {
                case IMGLOAD_FORMAT_B8G8R8A8:
                    convert_rgba_bgra(converted_out);
                    break;
                case IMGLOAD_FORMAT_R8G8B8:
                    convert_generic(img, data, IMGLOAD_FORMAT_R8G8B8A8, converted_out, IMGLOAD_FORMAT_R8G8B8, rgba_to_rgb);
                    break;
                case IMGLOAD_FORMAT_GRAY8:
                    // We can use rgb_to_luminance because the alpha value is ignored, we just need to set the right source format
                    convert_generic(img, data, IMGLOAD_FORMAT_R8G8B8A8, converted_out, IMGLOAD_FORMAT_GRAY8, rgb_to_luminance);
                    break;
                default:
                    err = IMGLOAD_ERR_UNSUPPORTED_CONVERSION;
                    break;
            }
            break;
        case IMGLOAD_FORMAT_B8G8R8A8:
            switch(destination)
            {
                case IMGLOAD_FORMAT_R8G8B8A8:
                    convert_rgba_bgra(converted_out);
                    break;
                case IMGLOAD_FORMAT_R8G8B8:
                    convert_rgba_bgra(data);
                    convert_generic(img, data, IMGLOAD_FORMAT_R8G8B8A8, converted_out, IMGLOAD_FORMAT_R8G8B8, rgba_to_rgb);
                    break;
                case IMGLOAD_FORMAT_GRAY8:
                    convert_rgba_bgra(data);
                    // We can use rgb_to_luminance because the alpha value is ignored, we just need to set the right source format
                    convert_generic(img, data, IMGLOAD_FORMAT_R8G8B8A8, converted_out, IMGLOAD_FORMAT_GRAY8, rgb_to_luminance);
                    break;
                default:
                    err = IMGLOAD_ERR_UNSUPPORTED_CONVERSION;
                    break;
            }
            break;
        case IMGLOAD_FORMAT_R8G8B8:
            switch(destination)
            {
                case IMGLOAD_FORMAT_R8G8B8A8:
                    convert_generic(img, data, IMGLOAD_FORMAT_R8G8B8, converted_out, IMGLOAD_FORMAT_R8G8B8A8, rgb_to_rgba);
                    break;
                case IMGLOAD_FORMAT_B8G8R8A8:
                    convert_generic(img, data, IMGLOAD_FORMAT_R8G8B8, converted_out, IMGLOAD_FORMAT_R8G8B8A8, rgb_to_rgba);
                    convert_rgba_bgra(converted_out);
                    break;
                case IMGLOAD_FORMAT_GRAY8:
                    convert_generic(img, data, IMGLOAD_FORMAT_R8G8B8, converted_out, IMGLOAD_FORMAT_GRAY8, rgb_to_luminance);
                    break;
                default:
                    err = IMGLOAD_ERR_UNSUPPORTED_CONVERSION;
                    break;
            }
            break;
        case IMGLOAD_FORMAT_GRAY8:
            switch(destination)
            {
                case IMGLOAD_FORMAT_R8G8B8A8:
                    convert_generic(img, data, IMGLOAD_FORMAT_GRAY8, converted_out, IMGLOAD_FORMAT_R8G8B8A8, luminance_to_rgba);
                    break;
                case IMGLOAD_FORMAT_B8G8R8A8:
                    convert_rgba_bgra(data);
                    convert_generic(img, data, IMGLOAD_FORMAT_GRAY8, converted_out, IMGLOAD_FORMAT_R8G8B8A8, luminance_to_rgba);
                    break;
                case IMGLOAD_FORMAT_R8G8B8:
                    convert_generic(img, data, IMGLOAD_FORMAT_GRAY8, converted_out, IMGLOAD_FORMAT_R8G8B8A8, luminance_to_rgb);
                    break;
                default:
                    err = IMGLOAD_ERR_UNSUPPORTED_CONVERSION;
                    break;
            }
            break;
        default:
            err = IMGLOAD_ERR_UNSUPPORTED_CONVERSION;
            break;
    }

    if (!in_place)
    {
        // Free the original data
        mem_free(img->context, data->data);
    }

    return err;
}

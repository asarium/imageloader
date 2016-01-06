#include <string.h>
#include "dxtc.h"
#include "util.h"
#include "memory.h"

const size_t DECOMPRESSED_BPP = 4;

size_t dxtc_get_data_size(DDSCompressedFormat_t format, uint32_t width, uint32_t height)
{
    size_t block_size = 0;
    switch (format)
    {
        case DDSIMG_COM_DXT1:
            block_size = 8;
            break;
        case DDSIMG_COM_DXT2:
        case DDSIMG_COM_DXT3:
        case DDSIMG_COM_DXT4:
        case DDSIMG_COM_DXT5:
            block_size = 16;
            break;
        case DDSIMG_COM_UNKNOWN:
            return 0;
    }

    uint32_t x_blocksize = MAX(1, (width + 3) / 4);
    uint32_t y_blocksize = MAX(1, (height + 3) / 4);

    return x_blocksize * y_blocksize * block_size;
}

static Color8888 dxtc_get_rgba_color(uint16_t color)
{
    Color8888 out;
    uint8_t r, g, b;

    b = (uint8_t) (color & 0x1f);
    g = (uint8_t) ((color & 0x7E0) >> 5);
    r = (uint8_t) ((color & 0xF800) >> 11);

    out.r = r << 3 | r >> 2;
    out.g = g << 2 | g >> 3;
    out.b = b << 3 | r >> 2;

    return out;
}

static void decompress_DXT1(MipmapData* compressed, MipmapData* decompressed)
{
    Color8888 colors[4];
    memset(colors, 0, sizeof(colors));

    size_t x;
    size_t y;
    size_t z;

    uint16_t color_0;
    uint16_t color_1;
    uint32_t bitmask;

    Color8888* data_out = (Color8888*) decompressed->data;
    uint8_t* data = (uint8_t*) compressed->data;

    size_t sizeof_plane = compressed->height * compressed->width;
    size_t sizeof_line = compressed->width;

    uint32_t dec_width = decompressed->width;
    uint32_t dec_height = decompressed->width;

    for (z = 0; z < compressed->depth; ++z)
    {
        for (y = 0; y < compressed->height; y += 4)
        {
            for (x = 0; x < compressed->width; x += 4)
            {
                color_0 = ((uint16_t*) data)[0];
                color_1 = ((uint16_t*) data)[1];

                colors[0] = dxtc_get_rgba_color(color_0);
                colors[1] = dxtc_get_rgba_color(color_1);
                bitmask = ((uint32_t*) data)[1];

                data += 8;

                if (color_0 > color_1)
                {
                    // Four-color block: derive the other two colors.
                    // 00 = color_0, 01 = color_1, 10 = color_2, 11 = color_3
                    // These 2-bit codes correspond to the 2-bit fields
                    // stored in the 64-bit block.
                    colors[2].b = (uint8_t) ((2 * colors[0].b + colors[1].b + 1) / 3);
                    colors[2].g = (uint8_t) ((2 * colors[0].g + colors[1].g + 1) / 3);
                    colors[2].r = (uint8_t) ((2 * colors[0].r + colors[1].r + 1) / 3);
                    colors[2].a = 0xFF;

                    colors[3].b = (uint8_t) ((colors[0].b + 2 * colors[1].b + 1) / 3);
                    colors[3].g = (uint8_t) ((colors[0].g + 2 * colors[1].g + 1) / 3);
                    colors[3].r = (uint8_t) ((colors[0].r + 2 * colors[1].r + 1) / 3);
                    colors[3].a = 0xFF;
                }
                else
                {
                    // Three-color block: derive the other color.
                    // 00 = color_0,  01 = color_1,  10 = color_2,
                    // 11 = transparent.
                    // These 2-bit codes correspond to the 2-bit fields
                    // stored in the 64-bit block.
                    colors[2].b = (uint8_t) ((colors[0].b + colors[1].b) / 2);
                    colors[2].g = (uint8_t) ((colors[0].g + colors[1].g) / 2);
                    colors[2].r = (uint8_t) ((colors[0].r + colors[1].r) / 2);
                    colors[2].a = 0xFF;

                    colors[3].b = (uint8_t) ((colors[0].b + 2 * colors[1].b + 1) / 3);
                    colors[3].g = (uint8_t) ((colors[0].g + 2 * colors[1].g + 1) / 3);
                    colors[3].r = (uint8_t) ((colors[0].r + 2 * colors[1].r + 1) / 3);
                    colors[3].a = 0x00;
                }

                uint32_t i, j, k, select;
                for (j = 0, k = 0; j < 4; j++)
                {
                    for (i = 0; i < 4; i++, k++)
                    {
                        select = (bitmask & (0x03 << k * 2)) >> k * 2;

                        size_t pixel_x = x + i;
                        // Decompressed data is flipped on the Y-axis, this accounts for that
                        size_t pixel_y = dec_height - (y + j) - 1;

                        size_t offset = sizeof_plane * z + sizeof_line * pixel_y + pixel_x;

                        if (pixel_x < dec_width && pixel_y < dec_height)
                        {
                            data_out[offset] = colors[select];
                        }
                    }
                }
            }
        }
    }
}

static void decompress_DXT3(MipmapData* compressed, MipmapData* decompressed)
{
    Color8888 colors[4];
    memset(colors, 0, sizeof(colors));

    uint8_t* alpha;

    size_t x;
    size_t y;
    size_t z;

    uint16_t color_0;
    uint16_t color_1;
    uint32_t bitmask;

    Color8888* data_out = (Color8888*) decompressed->data;
    uint8_t* data = (uint8_t*) compressed->data;

    size_t sizeof_plane = compressed->height * compressed->width;
    size_t sizeof_line = compressed->width;

    uint32_t dec_width = decompressed->width;
    uint32_t dec_height = decompressed->width;

    for (z = 0; z < compressed->depth; ++z)
    {
        for (y = 0; y < compressed->height; y += 4)
        {
            for (x = 0; x < compressed->width; x += 4)
            {
                alpha = data;
                data += 8; // 64 bits of alpha data

                color_0 = ((uint16_t*) data)[0];
                color_1 = ((uint16_t*) data)[1];

                colors[0] = dxtc_get_rgba_color(color_0);
                colors[1] = dxtc_get_rgba_color(color_1);
                bitmask = ((uint32_t*) data)[1];

                data += 8;

                // Four-color block: derive the other two colors.
                // 00 = color_0, 01 = color_1, 10 = color_2, 11 = color_3
                // These 2-bit codes correspond to the 2-bit fields
                // stored in the 64-bit block.
                colors[2].b = (uint8_t) ((2 * colors[0].b + colors[1].b + 1) / 3);
                colors[2].g = (uint8_t) ((2 * colors[0].g + colors[1].g + 1) / 3);
                colors[2].r = (uint8_t) ((2 * colors[0].r + colors[1].r + 1) / 3);
                colors[2].a = 0xFF;

                colors[3].b = (uint8_t) ((colors[0].b + 2 * colors[1].b + 1) / 3);
                colors[3].g = (uint8_t) ((colors[0].g + 2 * colors[1].g + 1) / 3);
                colors[3].r = (uint8_t) ((colors[0].r + 2 * colors[1].r + 1) / 3);
                colors[3].a = 0xFF;

                uint32_t i, j, k, select;
                uint16_t word;
                for (j = 0, k = 0; j < 4; j++)
                {
                    word = (uint16_t) (alpha[2 * j] + 256 * alpha[2 * j + 1]);
                    for (i = 0; i < 4; i++, k++)
                    {
                        select = (bitmask & (0x03 << k * 2)) >> k * 2;

                        size_t pixel_x = x + i;
                        // Decompressed data is flipped on the Y-axis, this accounts for that
                        size_t pixel_y = dec_height - (y + j) - 1;

                        uint8_t pixel_alpha = (uint8_t) (word & 0x0F);
                        pixel_alpha = pixel_alpha | pixel_alpha << 4;

                        size_t offset = sizeof_plane * z + sizeof_line * pixel_y + pixel_x;

                        if (pixel_x < dec_width && pixel_y < dec_height)
                        {
                            data_out[offset].r = colors[select].r;
                            data_out[offset].g = colors[select].g;
                            data_out[offset].b = colors[select].b;
                            data_out[offset].a = pixel_alpha;
                        }
                    }
                }
            }
        }
    }
}

static void decompress_DXT5(MipmapData* compressed, MipmapData* decompressed)
{
    Color8888 colors[4];
    memset(colors, 0, sizeof(colors));

    uint8_t alphas[8];
    uint64_t alphamask;

    size_t x;
    size_t y;
    size_t z;

    uint16_t color_0;
    uint16_t color_1;
    uint32_t bitmask;

    Color8888* data_out = (Color8888*) decompressed->data;
    uint8_t* data = (uint8_t*) compressed->data;

    size_t sizeof_plane = compressed->height * compressed->width;
    size_t sizeof_line = compressed->width;

    uint32_t dec_width = decompressed->width;
    uint32_t dec_height = decompressed->width;

    for (z = 0; z < compressed->depth; ++z)
    {
        for (y = 0; y < compressed->height; y += 4)
        {
            for (x = 0; x < compressed->width; x += 4)
            {
                alphas[0] = data[0];
                alphas[1] = data[1];

                // BIG ENDIAN WARNING! Won't work if system is big endian!
                alphamask = *((uint64_t*) (data + 2));
                // alphamask is 6 bytes long so we need to ignore the highest 2 bytes but the code below handles that

                data += 8;

                color_0 = ((uint16_t*) data)[0];
                color_1 = ((uint16_t*) data)[1];

                colors[0] = dxtc_get_rgba_color(color_0);
                colors[1] = dxtc_get_rgba_color(color_1);
                bitmask = ((uint32_t*) data)[1];

                data += 8;

                // Four-color block: derive the other two colors.
                // 00 = color_0, 01 = color_1, 10 = color_2, 11 = color_3
                // These 2-bit codes correspond to the 2-bit fields
                // stored in the 64-bit block.
                colors[2].b = (uint8_t) ((2 * colors[0].b + colors[1].b + 1) / 3);
                colors[2].g = (uint8_t) ((2 * colors[0].g + colors[1].g + 1) / 3);
                colors[2].r = (uint8_t) ((2 * colors[0].r + colors[1].r + 1) / 3);
                colors[2].a = 0xFF;

                colors[3].b = (uint8_t) ((colors[0].b + 2 * colors[1].b + 1) / 3);
                colors[3].g = (uint8_t) ((colors[0].g + 2 * colors[1].g + 1) / 3);
                colors[3].r = (uint8_t) ((colors[0].r + 2 * colors[1].r + 1) / 3);
                colors[3].a = 0xFF;

                // 8-alpha or 6-alpha block?
                if (alphas[0] > alphas[1]) {
                    // 8-alpha block:  derive the other six alphas.
                    // Bit code 000 = alpha_0, 001 = alpha_1, others are interpolated.
                    alphas[2] = (uint8_t)((6 * alphas[0] + 1 * alphas[1] + 3) / 7);	// bit code 010
                    alphas[3] = (uint8_t)((5 * alphas[0] + 2 * alphas[1] + 3) / 7);	// bit code 011
                    alphas[4] = (uint8_t)((4 * alphas[0] + 3 * alphas[1] + 3) / 7);	// bit code 100
                    alphas[5] = (uint8_t)((3 * alphas[0] + 4 * alphas[1] + 3) / 7);	// bit code 101
                    alphas[6] = (uint8_t)((2 * alphas[0] + 5 * alphas[1] + 3) / 7);	// bit code 110
                    alphas[7] = (uint8_t)((1 * alphas[0] + 6 * alphas[1] + 3) / 7);	// bit code 111
                }
                else {
                    // 6-alpha block.
                    // Bit code 000 = alpha_0, 001 = alpha_1, others are interpolated.
                    alphas[2] = (uint8_t)((4 * alphas[0] + 1 * alphas[1] + 2) / 5);	// Bit code 010
                    alphas[3] = (uint8_t)((3 * alphas[0] + 2 * alphas[1] + 2) / 5);	// Bit code 011
                    alphas[4] = (uint8_t)((2 * alphas[0] + 3 * alphas[1] + 2) / 5);	// Bit code 100
                    alphas[5] = (uint8_t)((1 * alphas[0] + 4 * alphas[1] + 2) / 5);	// Bit code 101
                    alphas[6] = 0x00;										// Bit code 110
                    alphas[7] = 0xFF;										// Bit code 111
                }

                uint32_t i, j, k, select;

                for (j = 0, k = 0; j < 4; j++)
                {
                    for (i = 0; i < 4; i++, k++)
                    {
                        select = (bitmask & (0x03 << k * 2)) >> k * 2;

                        size_t pixel_x = x + i;
                        // Decompressed data is flipped on the Y-axis, this accounts for that
                        size_t pixel_y = dec_height - (y + j) - 1;

                        size_t offset = sizeof_plane * z + sizeof_line * pixel_y + pixel_x;

                        if (pixel_x < dec_width && pixel_y < dec_height)
                        {
                            data_out[offset].r = colors[select].r;
                            data_out[offset].g = colors[select].g;
                            data_out[offset].b = colors[select].b;
                            data_out[offset].a = alphas[(uint8_t) (alphamask & 0x07)];
                        }

                        alphamask >>= 3;
                    }
                }
            }
        }
    }
}

static void correct_premult_alpha(MipmapData* decompressed)
{
    size_t i;
    uint8_t* data = decompressed->data;

    for (i = 0; i < decompressed->data_size; i += 4)
    {
        if (data[i + 3] != 0)
        {
            // Cannot divide by 0.
            data[i] = (uint8_t) (((uint32_t) data[i] << 8) / data[i + 3]);
            data[i + 1] = (uint8_t) (((uint32_t) data[i + 1] << 8) / data[i + 3]);
            data[i + 2] = (uint8_t) (((uint32_t) data[i + 2] << 8) / data[i + 3]);
        }
    }
}

DDSErrorCode dxtc_decrompress(DDSImage* img, MipmapData* compressed, MipmapData* decompressed)
{
    // We always decompress to RGBA with 8 bit for each channel
    size_t data_size = compressed->depth * compressed->width * compressed->height * DECOMPRESSED_BPP;
    decompressed->data = ddsimg_realloc(img->ctx, NULL, data_size);
    if (decompressed->data == NULL)
    {
        return DDSIMG_ERR_OUT_OF_MEMORY;
    }
    decompressed->data_size = data_size;

    decompressed->depth = compressed->depth;
    decompressed->height = compressed->height;
    decompressed->width = compressed->width;

    switch (img->props.compression_format)
    {
        case DDSIMG_COM_DXT1:
            decompress_DXT1(compressed, decompressed);
            break;
        case DDSIMG_COM_DXT2:
            decompress_DXT3(compressed, decompressed);
            correct_premult_alpha(decompressed);
            break;
        case DDSIMG_COM_DXT3:
            decompress_DXT3(compressed, decompressed);
            break;
        case DDSIMG_COM_DXT4:
            decompress_DXT5(compressed, decompressed);
            correct_premult_alpha(decompressed);
            break;
        case DDSIMG_COM_DXT5:
            decompress_DXT5(compressed, decompressed);
            break;
        case DDSIMG_COM_UNKNOWN:
            return DDSIMG_ERR_NOT_SUPPORTED;
    }

    return DDSIMG_ERR_NO_ERROR;
}

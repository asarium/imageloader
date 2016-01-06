#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include <ddsimg/ddsimg.h>

typedef struct
{
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t a;
} Color8888_t;

static size_t file_read(void* ud, uint8_t* buf, size_t buf_len)
{
    return fread(buf, 1, buf_len, (FILE*) ud);
}

static int64_t file_seek(void* ud, int64_t offset, int whence)
{
    if (fseek((FILE*) ud, offset, whence) != 0)
    {
        // Error
        return -1;
    }

    return (int64_t) ftell((FILE*) ud);
}

static void writeTGA(const char* name, uint32_t width, uint32_t height, void* data)
{
    Color8888_t* pixel_data = (Color8888_t*) data;

    FILE* outf = fopen(name, "wb");

    if (outf == NULL)
    {
        printf("Failed to open file %s!\n", name);
        return;
    }

    putc(0, outf);
    putc(0, outf);
    putc(2, outf);                         /* uncompressed RGB */
    putc(0, outf);
    putc(0, outf);
    putc(0, outf);
    putc(0, outf);
    putc(0, outf);
    putc(0, outf);
    putc(0, outf);           /* X origin */
    putc(0, outf);
    putc(0, outf);           /* y origin */
    putc((width & 0x00FF), outf);
    putc((width & 0xFF00) / 256, outf);
    putc((height & 0x00FF), outf);
    putc((height & 0xFF00) / 256, outf);
    putc(32, outf);                        /* 32 bit bitmap */
    putc(0, outf);

    uint32_t x, y;
    for (y = 0; y < height; ++y)
    {
        for (x = 0; x < width; ++x)
        {
            uint32_t offset = y * height + x;
            Color8888_t c = pixel_data[offset];

            putc(c.b, outf);
            putc(c.g, outf);
            putc(c.r, outf);
            putc(c.a, outf);
        }
    }

    fclose(outf);
}

int main(int argc, char** argv)
{
    if (argc != 2)
    {
        printf("Usage: dump_mipmaps <file_path>");
        return EXIT_FAILURE;
    }

    FILE* file = fopen(argv[1], "rb");

    DDSContext* ctx;

    if (ddsimg_context_alloc(&ctx, NULL, NULL) != DDSIMG_ERR_NO_ERROR)
    {
        printf("Failed to create dds context!");
        return EXIT_FAILURE;
    }

    DDSIOFunctions functions;
    functions.read = file_read;
    functions.seek = file_seek;

    DDSImage* img;
    if (ddsimg_image_alloc(ctx, &img, &functions, file) != DDSIMG_ERR_NO_ERROR)
    {
        printf("Failed to allocate image structure!");
        return EXIT_FAILURE;
    }

    if (ddsimg_image_read_header(img) != DDSIMG_ERR_NO_ERROR)
    {
        printf("Failed to read header!");
        return EXIT_FAILURE;
    }
    uint32_t width;
    uint32_t height;
    uint32_t depth;
    uint32_t mipmaps;
    uint32_t subimages;

    ddsimg_image_get_size(img, &width, &height, &depth);
    ddsimg_image_get_num_mipmaps(img, &mipmaps);
    ddsimg_image_get_num_subimages(img, &subimages);

    if (depth != 1)
    {
        printf("3D textures not supported!\n");
        return EXIT_FAILURE;
    }

    if (ddsimg_image_read_data(img) != DDSIMG_ERR_NO_ERROR)
    {
        printf("Failed to read data!\n");
    }
    char filename[255];
    uint32_t i, j;
    for (i = 0; i < subimages; ++i)
    {
        printf("Subimage: %u\n", i);
        for (j = 0; j < mipmaps; ++j)
        {
            printf("  Mipmap: %u\n", j);

            MipmapData data;

            DDSErrorCode err = ddsimg_image_get_decompressed_data(img, i, j, &data);

            if (err == DDSIMG_ERR_NO_DATA)
            {
                printf("No data for subimage %u, mipmap %u (probably a cubemap with missing faces)...\n", i, j);
            }
            if (err == DDSIMG_ERR_NO_ERROR)
            {
                snprintf(filename, sizeof(filename), "image-%u-%u.tga", i, j);
                writeTGA(filename, data.width, data.height, data.data);
            }
            else
            {
                printf("Failed to decompress subimage %u, mipmap %u!\n", i, j);
            }
        }
    }

    ddsimg_image_free(&img);

    ddsimg_context_free(&ctx);

    return EXIT_SUCCESS;
}

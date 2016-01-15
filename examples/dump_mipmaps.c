#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include <imageloader.h>

typedef struct
{
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t a;
} Color8888_t;

typedef struct
{
    uint8_t r;
    uint8_t g;
    uint8_t b;
} Color888_t;

static size_t IMGLOAD_CALLBACK file_read(void* ud, uint8_t* buf, size_t buf_len)
{
    return fread(buf, 1, buf_len, (FILE*) ud);
}

static int64_t IMGLOAD_CALLBACK file_seek(void* ud, int64_t offset, int whence)
{
    fseek((FILE*)ud, (long)offset, whence);

    return (int64_t) ftell((FILE*) ud);
}

static void* IMGLOAD_CALLBACK mem_realloc(void* ud, void* mem, size_t size)
{
    return realloc(mem, size);
}
static void IMGLOAD_CALLBACK mem_free(void* ud, void* mem)
{
    free(mem);
}

static void write24Bit(FILE* outf, ImgloadImageData* data)
{
    uint8_t* pixel_data = (uint8_t*)data->data;
    size_t x, y;
    for (y = 0; y < data->height; ++y)
    {
        for (x = 0; x < data->width; ++x)
        {
            size_t offset = y * data->stride + x * 3;
            Color888_t c = *(Color888_t*)(pixel_data + offset);

            putc(c.b, outf);
            putc(c.g, outf);
            putc(c.r, outf);
        }
    }
    
}

static void write32Bit(FILE* outf, ImgloadImageData* data)
{
    uint8_t* pixel_data = (uint8_t*)data->data;
    size_t x, y;
    for (y = 0; y < data->height; ++y)
    {
        for (x = 0; x < data->width; ++x)
        {
            size_t offset = y * data->stride + x * 4;
            Color8888_t c = *(Color8888_t*)(pixel_data + offset);

            putc(c.b, outf);
            putc(c.g, outf);
            putc(c.r, outf);
            putc(c.a, outf);
        }
    }
}

static void writeTGA(const char* name, ImgloadFormat format, ImgloadImageData* data)
{
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
    putc((uint32_t)(data->width & 0x00FF), outf);
    putc((uint32_t)(data->width & 0xFF00) / 256, outf);
    putc((uint32_t)(data->height & 0x00FF), outf);
    putc((uint32_t)(data->height & 0xFF00) / 256, outf);
    if (format == IMGLOAD_FORMAT_R8G8B8)
    {
        putc(24, outf);                        /* 24 bit bitmap */
    }
    else
    {
        putc(32, outf);                        /* 32 bit bitmap */
    }
    putc(32, outf);                     // Origin is top left

    if (format == IMGLOAD_FORMAT_R8G8B8)
    {
        write24Bit(outf, data);
    }
    else
    {
        write32Bit(outf, data);
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
    if (file == NULL)
    {
        printf("File could not be opened!");
        return EXIT_FAILURE;
    }

    ImgloadContext ctx;

    ImgloadMemoryAllocator allocator;
    allocator.realloc = mem_realloc;
    allocator.free = mem_free;

    if (imgload_context_init(&ctx, 0, &allocator, NULL) != IMGLOAD_ERR_NO_ERROR)
    {
        fclose(file);
        printf("Failed to create dds context!");
        return EXIT_FAILURE;
    }

    ImgloadIO functions;
    functions.read = file_read;
    functions.seek = file_seek;

    ImgloadImage img;
    if (imgload_image_init(ctx, &img, &functions, file) != IMGLOAD_ERR_NO_ERROR)
    {
        fclose(file);
        printf("Failed to allocate image structure!");
        return EXIT_FAILURE;
    }

    ImgloadFormat format = imgload_image_data_format(img);

    if (format != IMGLOAD_FORMAT_R8G8B8 && format != IMGLOAD_FORMAT_R8G8B8A8)
    {
        printf("Only RGB and RGBA images are currently supported!");
        return EXIT_FAILURE;
    }

    size_t subimages = imgload_image_num_subimages(img);

    if (imgload_image_read_data(img) != IMGLOAD_ERR_NO_ERROR)
    {
        printf("Failed to read data!\n");
        return EXIT_FAILURE;
    }
    char filename[255];
    uint32_t i, j;
    for (i = 0; i < subimages; ++i)
    {
        uint32_t width;
        uint32_t height;
        uint32_t depth;

        imgload_image_get_property(img, i, IMGLOAD_PROPERTY_WIDTH, IMGLOAD_PROPERTY_TYPE_UINT32, &width);
        imgload_image_get_property(img, i, IMGLOAD_PROPERTY_HEIGHT, IMGLOAD_PROPERTY_TYPE_UINT32, &height);
        imgload_image_get_property(img, i, IMGLOAD_PROPERTY_DEPTH, IMGLOAD_PROPERTY_TYPE_UINT32, &depth);

        if (depth != 1)
        {
            fclose(file);
            printf("3D textures not supported!\n");
            return EXIT_FAILURE;
        }

        size_t mipmaps = imgload_image_num_mipmaps(img, i);

        printf("Subimage: %u\n", i);
        for (j = 0; j < mipmaps; ++j)
        {
            printf("  Mipmap: %u\n", j);

            ImgloadImageData data;

            ImgloadErrorCode err = imgload_image_data(img, i, j, &data);

            if (err == IMGLOAD_ERR_NO_DATA)
            {
                printf("No data for subimage %u, mipmap %u (probably a cubemap with missing faces)...\n", i, j);
            }
            if (err == IMGLOAD_ERR_NO_ERROR)
            {
                snprintf(filename, sizeof(filename), "image-%u-%u.tga", i, j);
                writeTGA(filename, format, &data);
            }
            else
            {
                printf("Failed to decompress subimage %u, mipmap %u!\n", i, j);
            }
        }
    }

    imgload_image_free(img);

    imgload_context_free(ctx);

    fclose(file);
    return EXIT_SUCCESS;
}

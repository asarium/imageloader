#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include <imageloader.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

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

    if (format == IMGLOAD_FORMAT_B8G8R8A8)
    {
        imgload_image_transform_data(img, IMGLOAD_FORMAT_R8G8B8A8, 0);
        format = imgload_image_data_format(img);
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
                snprintf(filename, sizeof(filename), "image-%u-%u.png", i, j);
                int comp;
                switch(format)
                {
                    case IMGLOAD_FORMAT_R8G8B8A8:
                        comp = 4;
                        break;
                    case IMGLOAD_FORMAT_R8G8B8:
                        comp = 3;
                        break;
                    case IMGLOAD_FORMAT_GRAY8:
                        comp = 1;
                        break;
                    default:
                        printf("Unknown data format for subimage %u, mipmap %u!\n", i, j);
                        comp = 0;
                        break;
                }

                if (comp != 0)
                {
                    if (stbi_write_png(filename, (int) data.width, (int) data.height, comp, data.data, (int) data.stride) == 0)
                    {
                        printf("Failed to write image %s!\n", filename);
                    }
                }
            }
            else
            {
                printf("Failed to get data for subimage %u, mipmap %u!\n", i, j);
            }
        }
    }

    imgload_image_free(img);

    imgload_context_free(ctx);

    fclose(file);
    return EXIT_SUCCESS;
}

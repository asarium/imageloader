#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include <ddsimg/ddsimg.h>

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

int main(int argc, char** argv)
{
    if (argc != 2)
    {
        printf("Usage: dump_metadata <file_path>");
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
        printf("Failed to allocate image!");
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
    uint32_t compression;
    uint32_t format;
    uint32_t mipmaps;
    uint32_t subimages;
    uint32_t flags;

    ddsimg_image_get_size(img, &width, &height, &depth);
    ddsimg_image_get_compression(img, &compression);
    ddsimg_image_get_format(img, &format);
    ddsimg_image_get_num_mipmaps(img, &mipmaps);
    ddsimg_image_get_num_subimages(img, &subimages);
    ddsimg_image_get_flags(img, &flags);

    printf("Width  : %u\n", width);
    printf("Height : %u\n", height);
    printf("Depth  : %u\n", depth);

    switch (compression)
    {
        case DDSIMG_COM_DXT1:
            printf("Compr  : DXT1\n");
            break;
        case DDSIMG_COM_DXT2:
            printf("Compr  : DXT2\n");
            break;
        case DDSIMG_COM_DXT3:
            printf("Compr  : DXT3\n");
            break;
        case DDSIMG_COM_DXT4:
            printf("Compr  : DXT4\n");
            break;
        case DDSIMG_COM_DXT5:
            printf("Compr  : DXT5\n");
            break;
        default:
            printf("Compr  : Unknown\n");
            break;
    }

    switch (format)
    {
        case DDSIMG_FORMAT_R8G8B8A8:
            printf("Format : R8G8B8A8\n");
            break;
        default:
            printf("Format : Unknown\n");
            break;
    }

    printf("Flags  : %u\n", flags);
    printf("Subimgs: %u\n", subimages);
    printf("Mipmaps: %u\n", mipmaps);

    if (ddsimg_image_read_data(img) != DDSIMG_ERR_NO_ERROR)
    {
        printf("Failed to read data!\n");
    }

    printf("-------------- COMPRESSED:\n");
    uint32_t i, j;
    for (i = 0; i < subimages; ++i)
    {
        printf("Subimage: %u\n", i);
        for (j = 0; j < mipmaps; ++j)
        {
            printf("  Mipmap: %u\n", j);

            MipmapData mipmap;

            DDSErrorCode err = ddsimg_image_get_compressed_data(img, i, j, &mipmap);

            if (err != DDSIMG_ERR_NO_ERROR)
            {
                printf("Failed to get data!\n");
            }
            else
            {
                printf("    Width : %u\n", mipmap.width);
                printf("    Height: %u\n", mipmap.height);
                printf("    Depth : %u\n", mipmap.depth);

                printf("    Size  : %zu\n", mipmap.data_size);
                printf("    Ptr   : %p\n", mipmap.data);
            }
        }
    }

    printf("-------------- DECOMPRESSED:\n");
    for (i = 0; i < subimages; ++i)
    {
        printf("Subimage: %u\n", i);
        for (j = 0; j < mipmaps; ++j)
        {
            printf("  Mipmap: %u\n", j);

            MipmapData mipmap;

            DDSErrorCode err = ddsimg_image_get_decompressed_data(img, i, j, &mipmap);

            if (err != DDSIMG_ERR_NO_ERROR)
            {
                printf("Failed to get data!\n");
            }
            else
            {
                printf("    Width : %u\n", mipmap.width);
                printf("    Height: %u\n", mipmap.height);
                printf("    Depth : %u\n", mipmap.depth);

                printf("    Size  : %zu\n", mipmap.data_size);
                printf("    Ptr   : %p\n", mipmap.data);
            }
        }
    }

    ddsimg_image_free(&img);

    ddsimg_context_free(&ctx);

    return EXIT_SUCCESS;
}

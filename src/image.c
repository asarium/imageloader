#include "image.h"
#include "memory.h"
#include "plugin.h"
#include "context.h"

#include <string.h>
#include <assert.h>

ImgloadErrorCode IMGLOAD_API imgload_image_alloc(ImgloadContext ctx, ImgloadImage* image, ImgloadCustomIO* io,
                                                   void* io_ud)
{
    assert(ctx != NULL);
    assert(image != NULL);

    ImgloadImageImpl* img = mem_realloc(ctx, NULL, sizeof(ImgloadImageImpl));
    if (img == NULL)
    {
        return IMGLOAD_ERR_OUT_OF_MEMORY;
    }
    memset(img, 0, sizeof(*img));

    img->context = ctx;

    img->io.funcs = *io;
    img->io.ud = io_ud;

    *image = img;

    return IMGLOAD_ERR_NO_ERROR;
}

ImgloadErrorCode IMGLOAD_API imgload_read_header(ImgloadImage img)
{
    assert(img != NULL);

    ImgloadPlugin current = img->context->plugins.head;
    while(current != NULL)
    {
        int success;
        if (current->funcs.probe(current, &success) == IMGLOAD_ERR_NO_ERROR)
        {
            if (success)
            {
                // Found the right plugin, now initialize the plugin for this image
                ImgloadErrorCode err = current->funcs.init_image(current, img);

                if (err != IMGLOAD_ERR_NO_ERROR)
                {
                    return err;
                }
                img->plugin = current;

                break;
            }
        }
    }

    return IMGLOAD_UNSUPPORTED_FORMAT;
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

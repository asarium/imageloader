#include <imageloader.h>

#include "context.h"
#include "memory.h"
#include "log.h"

#include "project.h"

#include <string.h>
#include <assert.h>

#if WITH_LIBDDSIMG
#include "ddsimg.h"
#endif
#if WITH_PNG
#include "plugin_png.h"
#endif
#if WITH_STB_IMAGE
#include "plugin_stb_image.h"
#endif

static int register_default_plugins(ImgloadContext ctx)
{
#if WITH_LIBDDSIMG
    if (imgload_context_add_plugin(ctx, ddsimg_plugin_loader, NULL) != IMGLOAD_ERR_NO_ERROR)
    {
        print_to_log(ctx, IMGLOAD_LOG_ERROR, "Failed to initialize default libddsimg plugin!\n");
        return 0;
    }
#endif
#if WITH_PNG
    if (imgload_context_add_plugin(ctx, png_plugin_loader, NULL) != IMGLOAD_ERR_NO_ERROR)
    {
        print_to_log(ctx, IMGLOAD_LOG_ERROR, "Failed to initialize default png plugin!\n");
        return 0;
    }
#endif
#if WITH_STB_IMAGE
    if (imgload_context_add_plugin(ctx, stb_image_plugin_loader, NULL) != IMGLOAD_ERR_NO_ERROR)
    {
        print_to_log(ctx, IMGLOAD_LOG_ERROR, "Failed to initialize default stb_image plugin!\n");
        return 0;
    }
#endif

    return 1;
}

ImgloadErrorCode IMGLOAD_API imgload_context_init(ImgloadContext* ctx_ptr, ImgloadContextFlags flags,
                                                  ImgloadMemoryAllocator* allocator, void* alloc_ud)
{
    assert(ctx_ptr != NULL);
    assert(allocator != NULL);

    ImgloadContext ctx = allocator->realloc(alloc_ud, NULL, sizeof(struct ImgloadContextImpl));

    if (ctx == NULL)
    {
        return IMGLOAD_ERR_OUT_OF_MEMORY;
    }
    memset(ctx, 0, sizeof(*ctx));

    ctx->mem.allocator = *allocator;
    ctx->mem.ud = alloc_ud;
    ctx->flags = flags;

    ctx->log.minLevel = IMGLOAD_LOG_ERROR;

    if (!(flags & IMGLOAD_CONTEXT_NO_DEFAULT_PLUGINS))
    {
        if (!register_default_plugins(ctx))
        {
            imgload_context_free(ctx);
            return IMGLOAD_ERR_PLUGIN_ERROR;
        }
    }

    *ctx_ptr = ctx;

    return IMGLOAD_ERR_NO_ERROR;
}

ImgloadErrorCode IMGLOAD_API imgload_context_add_plugin(ImgloadContext ctx, ImgloadPluginLoader loader_func, void* plugin_param)
{
    assert(ctx != NULL);
    assert(loader_func != NULL);

    ImgloadPlugin plugin;
    
    ImgloadErrorCode err = plugin_init(ctx, loader_func, plugin_param, &plugin);

    if (err != IMGLOAD_ERR_NO_ERROR)
    {
        return err;
    }

    if (ctx->plugins.head == NULL)
    {
        // Initialize the list with the first plugin
        ctx->plugins.head = plugin;
        ctx->plugins.tail = plugin;
    }
    else
    {
        // Add the plugin to the end of the list
        plugin->prev = ctx->plugins.tail;

        ctx->plugins.tail->next = plugin;
        ctx->plugins.tail = plugin;
    }

    return IMGLOAD_ERR_NO_ERROR;
}

ImgloadErrorCode IMGLOAD_API imgload_context_set_log_callback(ImgloadContext ctx, ImgloadLogHandler handler, void* ud)
{
    assert(ctx != NULL);
    assert(handler != NULL);

    ctx->log.handler = handler;
    ctx->log.ud = ud;

    return IMGLOAD_ERR_NO_ERROR;
}

ImgloadErrorCode IMGLOAD_API imgload_context_set_log_level(ImgloadContext ctx, ImgloadLogLevel level)
{
    assert(ctx != NULL);

    ctx->log.minLevel = level;

    return IMGLOAD_ERR_NO_ERROR;
}

ImgloadErrorCode IMGLOAD_API imgload_context_free(ImgloadContext ctx)
{
    assert(ctx != NULL);

    // Free plugins
    while (ctx->plugins.tail != NULL)
    {
        ImgloadPlugin plugin = ctx->plugins.tail;
        ctx->plugins.tail = plugin->prev;
        plugin_free(plugin);
    }

    ctx->plugins.tail = NULL;
    ctx->plugins.head = NULL;

    mem_free(ctx, ctx);

    return IMGLOAD_ERR_NO_ERROR;
}

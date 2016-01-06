#include <imageloader.h>

#include "context.h"
#include "memory.h"

#include <string.h>
#include <assert.h>

ImgloadErrorCode IMGLOAD_API imgload_context_alloc(ImgloadContext* ctx_ptr, ImgloadMemoryAllocator* allocator,
                                                        void* alloc_ud)
{
    assert(ctx_ptr != NULL);
    assert(allocator != NULL);

    ImgloadContextImpl* ctx = allocator->realloc(alloc_ud, NULL, sizeof(ImgloadContextImpl));

    if (ctx == NULL)
    {
        return IMGLOAD_ERR_OUT_OF_MEMORY;
    }
    memset(ctx, 0, sizeof(*ctx));

    ctx->mem.allocator = *allocator;
    ctx->mem.ud = alloc_ud;

    *ctx_ptr = ctx;

    return IMGLOAD_ERR_NO_ERROR;
}

ImgloadErrorCode IMGLOAD_API imgload_context_add_plugin(ImgloadContext ctx, ImgloadPluginLoader loader_func, void* plugin_param)
{
    assert(ctx != NULL);
    assert(loader_func != NULL);

    ImgloadPluginImpl* plugin;
    
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

ImgloadErrorCode imgload_context_set_log_callback(ImgloadContext ctx, ImgloadLogHandler handler, void* ud)
{
    assert(ctx != NULL);
    assert(handler != NULL);

    ctx->log.handler = handler;
    ctx->log.ud = ud;

	return IMGLOAD_ERR_NO_ERROR;
}

const char* IMGLOAD_API imgload_context_get_error(ImgloadContext ctx)
{
    return ctx->last_error;
}

ImgloadErrorCode IMGLOAD_API imgload_context_free(ImgloadContext ctx)
{
    assert(ctx != NULL);

    // Free plugins
    while (ctx->plugins.tail != NULL)
    {
        ImgloadPluginImpl* plugin = ctx->plugins.tail;
        ctx->plugins.tail = plugin->prev;
        plugin_free(plugin);
    }

    ctx->plugins.tail = NULL;
    ctx->plugins.head = NULL;

    mem_free(ctx, ctx);

    return IMGLOAD_ERR_NO_ERROR;
}

void ctx_set_last_error(ImgloadContext ctx, const char* err)
{
    ctx->last_error = err;
}

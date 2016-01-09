#include "plugin.h"
#include "memory.h"
#include "log.h"

#include <string.h>
#include <assert.h>

static ImgloadErrorCode validate_plugin(ImgloadPlugin plugin)
{
    if (plugin->info.id == NULL)
    {
        print_to_log(plugin->context, IMGLOAD_LOG_ERROR, "A plugin has to have an identification string!\n");
        return IMGLOAD_ERR_PLUGIN_INVALID;
    }

    if (plugin->funcs.probe == NULL)
    {
        print_to_log(plugin->context, IMGLOAD_LOG_ERROR, "[%s] A plugin has to have a probe function!\n", plugin->info.id);
        return IMGLOAD_ERR_PLUGIN_INVALID;
    }
    if (plugin->funcs.init_image == NULL)
    {
        print_to_log(plugin->context, IMGLOAD_LOG_ERROR, "[%s] A plugin has to have an image init function!\n", plugin->info.id);
        return IMGLOAD_ERR_PLUGIN_INVALID;
    }

    return IMGLOAD_ERR_NO_ERROR;
}

ImgloadErrorCode plugin_init(ImgloadContext ctx, ImgloadPluginLoader loader, void* param, ImgloadPlugin* plugin_out)
{
    assert(plugin_out != NULL);

    *plugin_out = NULL;

    ImgloadPlugin plugin = mem_reallocz(ctx, NULL, sizeof(struct ImgloadPluginImpl));
    if (plugin == NULL)
    {
        return IMGLOAD_ERR_OUT_OF_MEMORY;
    }
    plugin->context = ctx;

    ImgloadErrorCode err = loader(plugin, param);

    if (err != IMGLOAD_ERR_NO_ERROR)
    {
        mem_free(ctx, plugin);
        return err;
    }

    err = validate_plugin(plugin);
    if (err != IMGLOAD_ERR_NO_ERROR)
    {
        plugin_free(plugin);
        return err;
    }

    *plugin_out = plugin;
    return IMGLOAD_ERR_NO_ERROR;
}

void plugin_free(ImgloadPlugin plugin)
{
    assert(plugin != NULL);

    if (plugin->funcs.deinit)
    {
        plugin->funcs.deinit(plugin);
    }

    mem_free(plugin->context, plugin);
}

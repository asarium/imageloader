
#include <imageloader.h>
#include <imageloader_plugin.h>

#include "image.h"
#include "plugin.h"
#include "memory.h"
#include "log.h"

#include <stdio.h>
#include <stdarg.h>
#include <assert.h>

size_t IMGLOAD_API imgload_plugin_read(ImgloadPlugin plugin, ImgloadImage img, uint8_t* buf, size_t size)
{
    assert(plugin != NULL);
    assert(plugin == img->plugin);

    return image_io_read(img, buf, size);
}

int64_t IMGLOAD_API imgload_plugin_seek(ImgloadPlugin plugin, ImgloadImage img, int64_t offset, int whence)
{
    assert(plugin != NULL);
    assert(plugin == img->plugin);

    return image_io_seek(img, offset, whence);
}


void* IMGLOAD_API imgload_plugin_realloc(ImgloadPlugin plugin, void* ptr, size_t size)
{
    assert(plugin != NULL);

    return mem_realloc(plugin->context, ptr, size);
}

void IMGLOAD_API imgload_plugin_free(ImgloadPlugin plugin, void* ptr)
{
    assert(plugin != NULL);

    mem_free(plugin->context, ptr);
}

void imgload_plugin_log(ImgloadPlugin plugin, ImgloadLogLevel level, const char* format, ...)
{
	char buffer[1024];

	va_list list;
	va_start(list, format);

	int ret = vsnprintf(buffer, sizeof(buffer) / sizeof(buffer[0]), format, list);

	if (ret < 0)
	{
		print_to_log(plugin->context, level, "[%s] Failed to evaluate format string '%s'\n", plugin->info.id, format);
	}

	print_to_log(plugin->context, level, "[%s] %s\n", plugin->info.id, buffer);

	va_end(list);
}

void imgload_plugin_set_data(ImgloadPlugin plugin, void* data)
{
    assert(plugin != NULL);

    plugin->plugin_data = data;
}

void* imgload_plugin_get_data(ImgloadPlugin plugin)
{
    assert(plugin != NULL);

    return plugin->plugin_data;
}

void imgload_plugin_set_info(ImgloadPlugin plugin, const char* id, const char* name, const char* description)
{
	assert(plugin != NULL);

	plugin->info.id = id;
	plugin->info.name = name;
	plugin->info.description = description;
}

void imgload_plugin_callback_deinit(ImgloadPlugin plugin, ImgloadPluginDeinitFunc func)
{
    assert(plugin != NULL);
    assert(func != NULL);

    plugin->funcs.deinit = func;
}

void IMGLOAD_API imgload_plugin_callback_probe(ImgloadPlugin plugin, ImgloadPluginProbeFunc func)
{
    assert(plugin != NULL);
    assert(func != NULL);

    plugin->funcs.probe = func;
}

void IMGLOAD_API imgload_plugin_callback_init_image(ImgloadPlugin plugin, ImgloadPluginImageFunc func)
{
    assert(plugin != NULL);
    assert(func != NULL);

    plugin->funcs.init_image = func;
}

void IMGLOAD_API imgload_plugin_callback_deinit_image(ImgloadPlugin plugin, ImgloadPluginImageFunc func)
{
    assert(plugin != NULL);
    assert(func != NULL);

    plugin->funcs.deinit_image = func;
}


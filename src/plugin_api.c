
#include <imageloader.h>
#include <imageloader_plugin.h>

#include "image.h"
#include "plugin.h"
#include "memory.h"
#include "log.h"

#include <stdio.h>
#include <stdarg.h>
#include <assert.h>

size_t IMGLOAD_API imgload_plugin_image_read(ImgloadImage img, uint8_t* buf, size_t size)
{
    assert(img != NULL);

    return image_io_read(img, buf, size);
}

int64_t IMGLOAD_API imgload_plugin_image_seek(ImgloadImage img, int64_t offset, int whence)
{
    assert(img != NULL);

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

void IMGLOAD_API imgload_plugin_log(ImgloadPlugin plugin, ImgloadLogLevel level, const char* format, ...)
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

void IMGLOAD_API imgload_plugin_set_data(ImgloadPlugin plugin, void* data)
{
    assert(plugin != NULL);

    plugin->plugin_data = data;
}

void* IMGLOAD_API imgload_plugin_get_data(ImgloadPlugin plugin)
{
    assert(plugin != NULL);

    return plugin->plugin_data;
}

void IMGLOAD_API imgload_plugin_set_info(ImgloadPlugin plugin, const char* id, const char* name, const char* description)
{
    assert(plugin != NULL);

    plugin->info.id = id;
    plugin->info.name = name;
    plugin->info.description = description;
}

void IMGLOAD_API imgload_plugin_callback_deinit(ImgloadPlugin plugin, ImgloadPluginDeinitFunc func)
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

void IMGLOAD_API imgload_plugin_callback_read_data(ImgloadPlugin plugin, ImgloadPluginImageFunc func)
{
    assert(plugin != NULL);
    assert(func != NULL);

    plugin->funcs.read_image = func;
}

void IMGLOAD_API imgload_plugin_callback_decompress_data(ImgloadPlugin plugin, ImgloadPluginDecompressData func)
{
    assert(plugin != NULL);
    assert(func != NULL);

    plugin->funcs.decompress_data = func;
}

void IMGLOAD_API imgload_plugin_image_set_data(ImgloadImage img, void* data)
{
    assert(img != NULL);

    img->plugin_data = data;
}
void* IMGLOAD_API imgload_plugin_image_get_data(ImgloadImage img)
{
    assert(img != NULL);

    return img->plugin_data;
}

ImgloadErrorCode IMGLOAD_API imgload_plugin_image_set_num_frames(ImgloadImage img, size_t num_frames)
{
    return image_allocate_frames(img, num_frames);
}

void IMGLOAD_API imgload_plugin_image_set_data_type(ImgloadImage img, ImgloadFormat format, ImgloadCompression compreesion)
{
    assert(img != NULL);

    img->data_format = format;
    img->data_format_initialized = true;

    img->compression = compreesion;
    img->compression_initialized = true;
}

ImgloadErrorCode IMGLOAD_API imgload_plugin_image_set_property(ImgloadImage img, size_t subimage,
    ImgloadProperty prop, ImgloadPropertyType type, void* val)
{
    assert(img != NULL);

    if (subimage >= img->n_frames)
    {
        return IMGLOAD_ERR_OUT_OF_RANGE;
    }

    PropertyValue* property = &img->frames[subimage].properties[prop];

    if (property->initialized && type != property->type)
    {
        print_to_log(img->context, IMGLOAD_LOG_WARNING, "Plugin tried to change type of a property!\n");
        return IMGLOAD_ERR_WRONG_TYPE;
    }

    switch (type)
    {
    case IMGLOAD_PROPERTY_TYPE_UINT32:
        property->value.uint32 = *(uint32_t*)val;
        break;
    case IMGLOAD_PROPERTY_TYPE_INT32:
        property->value.int32 = *(int32_t*)val;
        break;
    case IMGLOAD_PROPERTY_TYPE_FLAOT:
        property->value.float_val = *(float*)val;
        break;
    case IMGLOAD_PROPERTY_TYPE_DOUBLE:
        property->value.double_val = *(double*)val;
        break;
    case IMGLOAD_PROPERTY_TYPE_STRING:
        property->value.str = mem_strdup(img->context, *(const char**)val);
        break;
    case IMGLOAD_PROPERTY_TYPE_COMPLEX:
        property->value.complex = *(void**)val;
        break;
    }
    property->type = IMGLOAD_PROPERTY_TYPE_UINT32;
    property->initialized = true;

    return IMGLOAD_ERR_NO_ERROR;
}

ImgloadErrorCode IMGLOAD_API imgload_plugin_image_set_num_mipmaps(ImgloadImage img, size_t subimage, size_t mipmaps)
{
    return image_allocate_mipmaps(img, subimage, mipmaps);
}

void IMGLOAD_API imgload_plugin_image_set_compressed_data(ImgloadImage img, size_t subimage, size_t mipmap,
ImgloadImageData* data, int transfer_ownership)
{
    assert(img != NULL);
    assert(subimage < img->n_frames);
    assert(mipmap < img->frames[subimage].n_mipmaps);

    image_set_compressed_data(img, subimage, mipmap, data, transfer_ownership != 0);
}

void IMGLOAD_API imgload_plugin_image_set_image_data(ImgloadImage img, size_t subimage, size_t mipmap,
    ImgloadImageData* data, int transfer_ownership)
{
    assert(img != NULL);
    assert(subimage < img->n_frames);
    assert(mipmap < img->frames[subimage].n_mipmaps);

    image_set_data(img, subimage, mipmap, data, transfer_ownership != 0);
}

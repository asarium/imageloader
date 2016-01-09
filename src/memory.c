#include <imageloader.h>

#include "context.h"
#include "memory.h"
#include <string.h>

void* mem_realloc(ImgloadContext ctx, void* ptr, size_t size)
{
    return ctx->mem.allocator.realloc(ctx->mem.ud, ptr, size);
}

void* mem_reallocz(ImgloadContext ctx, void* ptr, size_t size)
{
    void* data = mem_realloc(ctx, ptr, size);

    if (data != NULL)
    {
        memset(data, 0, size);
    }

    return data;
}

void mem_free(ImgloadContext ctx, void* ptr)
{
    ctx->mem.allocator.free(ctx->mem.ud, ptr);
}

char* mem_strdup(ImgloadContext ctx, const char* str)
{
    size_t len = strlen(str);

    char* new_mem = (char*)mem_realloc(ctx, NULL, len + 1);

    if (new_mem == NULL)
    {
        // Memory allocation failure
        return NULL;
    }
    strncpy(new_mem, str, len);
    new_mem[len] = '\0';

    return new_mem;
}

#include <imageloader.h>

#include "context.h"
#include "memory.h"

void* mem_realloc(ImgloadContext ctx, void* ptr, size_t size)
{
    return ctx->mem.allocator.realloc(ctx->mem.ud, ptr, size);
}

void mem_free(ImgloadContext ctx, void* ptr)
{
    ctx->mem.allocator.free(ctx->mem.ud, ptr);
}

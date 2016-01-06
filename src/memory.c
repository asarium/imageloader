#include "memory.h"
#include "context.h"

#include <assert.h>
#include <stdlib.h>

#include <ddsimg/ddsimg.h>

static void* DDSIMG_CC std_realloc(void* ud, void* ptr, size_t size)
{
    return realloc(ptr, size);
}

static void DDSIMG_CC std_free(void* ud, void* ptr)
{
    free(ptr);
}

static DDSMemoryFunctions std_mem_funcs = {
        std_realloc,
        std_free
};

void* ddsimg_realloc(DDSContext* ctx, void* ptr, size_t size)
{
    assert(ctx != NULL);
    return ctx->mem.funcs.realloc(ctx->mem.ud, ptr, size);
}

void ddsimg_free(DDSContext* ctx, void* ptr)
{
    assert(ctx != NULL);

    ctx->mem.funcs.free(ctx->mem.ud, ptr);
}

DDSMemoryFunctions* ddsimg_mem_default_memfuncs()
{
    return &std_mem_funcs;
}

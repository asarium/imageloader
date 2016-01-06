#include <ddsimg/ddsimg.h>
#include <string.h>
#include <assert.h>

#include "context.h"
#include "memory.h"

DDSErrorCode DDSIMG_API ddsimg_context_alloc(DDSContext** contextPtr, DDSMemoryFunctions* memFuncs, void* mem_ud)
{
    if (contextPtr == NULL)
    {
        return DDSIMG_ERR_INVALID_API_USAGE;
    }

    if (memFuncs == NULL)
    {
        memFuncs = ddsimg_mem_default_memfuncs();
    }

    DDSContext* ctx = memFuncs->realloc(mem_ud, NULL, sizeof(DDSContext));

    if (ctx == NULL)
    {
        return DDSIMG_ERR_OUT_OF_MEMORY;
    }
    memset(ctx, 0, sizeof(*ctx));

    // set the out parameter
    *contextPtr = ctx;

    ctx->mem.funcs = *memFuncs;
    ctx->mem.ud = mem_ud;

    return DDSIMG_ERR_NO_ERROR;
}

DDSErrorCode DDSIMG_API ddsimg_context_free(DDSContext** ctx)
{
    if (ctx == NULL)
    {
        return DDSIMG_ERR_INVALID_API_USAGE;
    }

    ddsimg_free(*ctx, *ctx);
    *ctx = NULL;

    return DDSIMG_ERR_NO_ERROR;
}

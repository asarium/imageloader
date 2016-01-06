
#include <gtest/gtest.h>

#include <ddsimg/ddsimg.h>

namespace
{
    void* DDSIMG_CALLBACK no_memory_realloc(void*, void*, size_t)
    {
        return NULL;
    }

	void DDSIMG_CALLBACK no_memory_free(void*, void*)
    {
    }
}

TEST(ContextTests, allocate_free)
{
    ASSERT_EQ(DDSIMG_ERR_INVALID_API_USAGE, ddsimg_context_alloc(NULL, NULL, NULL));

    DDSContext* ctx;
    DDSMemoryFunctions no_memory_funcs;
    no_memory_funcs.realloc = no_memory_realloc;
    no_memory_funcs.free = no_memory_free;

    ASSERT_EQ(DDSIMG_ERR_OUT_OF_MEMORY, ddsimg_context_alloc(&ctx, &no_memory_funcs, NULL));

    ASSERT_EQ(DDSIMG_ERR_NO_ERROR, ddsimg_context_alloc(&ctx, NULL, NULL));

    ASSERT_EQ(DDSIMG_ERR_NO_ERROR, ddsimg_context_free(&ctx));

    ASSERT_EQ(DDSIMG_ERR_INVALID_API_USAGE, ddsimg_context_free(NULL));
}

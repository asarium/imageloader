
#include "util.h"

#include <cstdio>

namespace
{
    void* IMGLOAD_CALLBACK mem_realloc(void* ud, void* mem, size_t size)
    {
        return realloc(mem, size);
    }
    void IMGLOAD_CALLBACK mem_free(void* ud, void* mem)
    {
        free(mem);
    }

    size_t IMGLOAD_CALLBACK std_read(void* ud, uint8_t* buf, size_t size)
    {
        return std::fread(buf, 1, size, static_cast<FILE*>(ud));
    }

    int64_t IMGLOAD_CALLBACK std_seek(void* ud, int64_t offset, int whence)
    {
        std::fseek(static_cast<FILE*>(ud), static_cast<long>(offset), whence);
        return static_cast<int64_t>(std::ftell(static_cast<FILE*>(ud)));
    }
}

namespace util
{
    void ContextFixture::SetUp()
    {
        ImgloadMemoryAllocator allocator;
        allocator.realloc = mem_realloc;
        allocator.free = mem_free;

        auto err = imgload_context_alloc(&ctx, 0, &allocator, nullptr);

        if (err != IMGLOAD_ERR_NO_ERROR)
        {
            ctx = nullptr;
            FAIL() << "Failed to allocate imgload context!";
        }
    }

    void ContextFixture::TearDown()
    {
        if (ctx != nullptr)
        {
            imgload_context_free(ctx);
            ctx = nullptr;
        }
    }

    ImgloadIO get_std_io()
    {
        ImgloadIO io;
        io.read = std_read;
        io.seek = std_seek;

        return io;
    }
}


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

    ImgloadErrorCode IMGLOAD_CALLBACK logger(void* ud, ImgloadLogLevel level, const char* text)
    {
        switch(level)
        {
        case IMGLOAD_LOG_DEBUG:
            std::cout << "DEBG: ";
            break;
        case IMGLOAD_LOG_INFO:
            std::cout << "INFO: ";
            break;
        case IMGLOAD_LOG_WARNING:
            std::cout << "WARN: ";
            break;
        case IMGLOAD_LOG_ERROR:
            std::cout << " ERR: ";
            break;
        }

        std::cout << text;

        return IMGLOAD_ERR_NO_ERROR;
    }
}

namespace util
{
    void ContextFixture::SetUp()
    {
        ImgloadMemoryAllocator allocator;
        allocator.realloc = mem_realloc;
        allocator.free = mem_free;

        auto err = imgload_context_init(&ctx, 0, &allocator, nullptr);

        if (err != IMGLOAD_ERR_NO_ERROR)
        {
            ctx = nullptr;
            FAIL() << "Failed to allocate imgload context!";
        }

        imgload_context_set_log_callback(ctx, logger, nullptr);
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

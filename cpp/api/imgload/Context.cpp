#include <imageloader.h>

#include "Exception.hpp"
#include "Context.hpp"
#include "Image.hpp"

namespace
{
    void* IMGLOAD_CALLBACK class_realloc(void* ud, void* mem, size_t size)
    {
        return reinterpret_cast<imgload::MemoryAllocator*>(ud)->reallocate(mem, size);
    }

    void IMGLOAD_CALLBACK class_free(void* ud, void* mem)
    {
        return reinterpret_cast<imgload::MemoryAllocator*>(ud)->free(mem);
    }

    ImgloadErrorCode IMGLOAD_CALLBACK class_log(void* ud, ImgloadLogLevel level, const char* message)
    {
        using namespace imgload;

        LogLevel logLevel;
        switch (level)
        {
            case IMGLOAD_LOG_DEBUG:
                logLevel = LogLevel::DEBUG;
                break;
            case IMGLOAD_LOG_INFO:
                logLevel = LogLevel::INFO;
                break;
            case IMGLOAD_LOG_WARNING:
                logLevel = LogLevel::WARNING;
                break;
            case IMGLOAD_LOG_ERROR:
                logLevel = LogLevel::ERROR;
                break;
            default:
                logLevel = LogLevel::ERROR;
                break;
        }

        reinterpret_cast<imgload::Logger*>(ud)->log(logLevel, message);

        return IMGLOAD_ERR_NO_ERROR;
    }

    size_t IMGLOAD_CALLBACK class_read(void* ud, uint8_t* buf, size_t size)
    {
        return reinterpret_cast<imgload::IOHandler*>(ud)->read(buf, size);
    }

    int64_t IMGLOAD_CALLBACK class_seek(void* ud, int64_t offset, int whence)
    {
        return reinterpret_cast<imgload::IOHandler*>(ud)->seek(offset, whence);
    }
}

namespace imgload
{

    Context::Context(std::unique_ptr<MemoryAllocator>&& allocator) : m_ctx(nullptr), m_allocator(std::move(allocator))
    {
        ImgloadMemoryAllocator alloc;
        alloc.realloc = class_realloc;
        alloc.free = class_free;

        auto err = imgload_context_init(&m_ctx, 0, &alloc, m_allocator.get());

        if (err != IMGLOAD_ERR_NO_ERROR)
        {
            throw Exception(err);
        }
    }


    Context::~Context()
    {
        if (m_ctx)
        {
            imgload_context_free(m_ctx);
            m_ctx = nullptr;
        }
    }

    Context::Context(Context&& other)
    {
        *this = std::move(other);
    }

    Context& Context::operator=(Context&& other)
    {
        std::swap(m_ctx, other.m_ctx);
        std::swap(m_allocator, other.m_allocator);

        return *this;
    }

    void Context::setLogger(std::unique_ptr<Logger>&& logger)
    {
        if (!m_ctx)
        {
            throw std::runtime_error("No context allocated!");
        }

        auto err = imgload_context_set_log_callback(m_ctx, class_log, reinterpret_cast<void*>(logger.get()));

        if (err != IMGLOAD_ERR_NO_ERROR)
        {
            throw Exception(err);
        }

        m_logger = std::move(logger);
    }

    Image Context::loadImage(std::unique_ptr<IOHandler>&& io)
    {
        ImgloadIO img_io;
        img_io.read = class_read;
        img_io.seek = class_seek;

        ImgloadImage image;
        auto err = imgload_image_init(m_ctx, &image, &img_io, reinterpret_cast<void*>(io.get()));
        if (err != IMGLOAD_ERR_NO_ERROR)
        {
            throw Exception(err);
        }

        // Pass io to the Image instance so it's kept alive for as long as the Image exists
        return Image(image, std::move(io));
    }
}
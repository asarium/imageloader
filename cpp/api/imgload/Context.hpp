#pragma once

#include <memory>

#include <imageloader.h>

namespace imgload
{
    class MemoryAllocator
    {
    public:
        virtual ~MemoryAllocator() {}

        virtual void* reallocate(void* mem, size_t size) = 0;

        virtual void free(void* mem) = 0;
    };

    enum class LogLevel
    {
        DEBUG,
        INFO,
        WARNING,
        ERROR,
    };

    class Logger
    {
    public:
        virtual ~Logger() {}

        virtual void log(LogLevel level, const char* text) = 0;
    };

    class IOHandler
    {
    public:
        virtual ~IOHandler() {}

        virtual size_t read(uint8_t* buf, size_t size) = 0;

        virtual int64_t seek(int64_t offset, int whence) = 0;
    };

    class Image;

    class Context
    {
    private:
        ImgloadContext m_ctx;

        std::unique_ptr<MemoryAllocator> m_allocator;
        std::unique_ptr<Logger> m_logger;

    public:
        explicit Context(std::unique_ptr<MemoryAllocator>&& allocator);

        Context(Context&& other);

        ~Context();

        Context& operator=(Context&& other);

        // This class is not copy-able
        Context(const Context&) = delete;

        // This class is not copy-able
        Context& operator=(const Context&) = delete;

        void setLogger(std::unique_ptr<Logger>&& logger);

        Image loadImage(std::unique_ptr<IOHandler>&& io);
    };
}




#pragma once

#include "Context.hpp"

#include <cstdint>

#include <imageloader.h>

typedef struct ImgloadImageImpl* ImgloadImage;

namespace imgload
{
    enum class DataFormat
    {
        R8G8B8A8,
        R8G8B8,
        GRAY8,
    };

    enum class Compression
    {
        NONE,
        DXT1,
        DXT2,
        DXT3,
        DXT4,
        DXT5,
    };

    enum class Property
    {
        WIDTH,
        HEIGHT,
        DEPTH,
        PLUGIN_DATA_1,
        PLUGIN_DATA_2,
        PLUGIN_DATA_3,
        PLUGIN_DATA_4,
    };

    class SubImage
    {
        ImgloadImage m_image;
        size_t m_index;

        enum class Type
        {
            Int32,
            Uint32,
            Float,
            Double,
            String,
            Complex
        };

        template<typename T>
        Type getPropertyType();

        void getPropertyInternal(Property prop, Type type, void* val_out);

    private:
        SubImage(ImgloadImage image, size_t i);

    public:
        ~SubImage()
        { }

        size_t numMipmaps();

        template<typename T>
        T getProperty(Property prop)
        {
            T val;
            getPropertyInternal(prop, getPropertyType<T>(), &val);
            return val;
        }

        ImgloadImageData getCompressedData(size_t mipmap);

        ImgloadImageData getImageData(size_t mipmap);

        friend class Image;
    };

    template<typename T>
    SubImage::Type SubImage::getPropertyType()
    {
        // The static assert is always false to cause an error when the property type is not known
        static_assert(sizeof(T) == 0, "Unsupported property type!");
        return Type::Complex;
    }

    template<>
    SubImage::Type SubImage::getPropertyType<std::int32_t>()
    {
        return Type::Int32;
    }

    template<>
    SubImage::Type SubImage::getPropertyType<std::uint32_t>()
    {
        return Type::Uint32;
    }

    template<>
    SubImage::Type SubImage::getPropertyType<float>()
    {
        return Type::Float;
    }

    template<>
    SubImage::Type SubImage::getPropertyType<double>()
    {
        return Type::Double;
    }

    template<>
    SubImage::Type SubImage::getPropertyType<const char*>()
    {
        return Type::String;
    }

    template<>
    SubImage::Type SubImage::getPropertyType<void*>()
    {
        return Type::Complex;
    }

    class Image
    {
        ImgloadImage m_image;

        std::unique_ptr<IOHandler> m_io;

        Image(ImgloadImage image, std::unique_ptr<IOHandler>&& io_handler);

    public:

        Image(Image&& other);

        ~Image();

        Image& operator=(Image&& other);

        // This class is not copy-able
        Image(const Image&) = delete;

        // This class is not copy-able
        Image& operator=(const Image&) = delete;

        DataFormat getFormat() const;

        Compression getCompression() const;

        size_t numSubimages() const;

        SubImage getSubimage(size_t index) const;

        void readData();

        friend class Context;
    };
}



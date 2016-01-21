#include <imageloader.h>

#include "Image.hpp"
#include "Exception.hpp"

#include <stdexcept>

namespace
{
    using namespace imgload;

    ImgloadProperty convertProperty(imgload::Property prop)
    {
        switch (prop)
        {
            case imgload::Property::WIDTH:
                return IMGLOAD_PROPERTY_WIDTH;
            case imgload::Property::HEIGHT:
                return IMGLOAD_PROPERTY_HEIGHT;
            case imgload::Property::DEPTH:
                return IMGLOAD_PROPERTY_DEPTH;
            case imgload::Property::PLUGIN_DATA_1:
                return IMGLOAD_PROPERTY_PLUGIN_DATA_1;
            case imgload::Property::PLUGIN_DATA_2:
                return IMGLOAD_PROPERTY_PLUGIN_DATA_2;
            case imgload::Property::PLUGIN_DATA_3:
                return IMGLOAD_PROPERTY_PLUGIN_DATA_3;
            case imgload::Property::PLUGIN_DATA_4:
                return IMGLOAD_PROPERTY_PLUGIN_DATA_4;
        }
        return IMGLOAD_PROPERTY_WIDTH;
    }

    ImgloadFormat imgloadFormat(imgload::DataFormat format)
    {
        switch (format)
        {
            case DataFormat::R8G8B8A8:
                return IMGLOAD_FORMAT_R8G8B8A8;
            case DataFormat::B8G8R8A8:
                return IMGLOAD_FORMAT_B8G8R8A8;
            case DataFormat::R8G8B8:
                return IMGLOAD_FORMAT_R8G8B8;
            case DataFormat::GRAY8:
                return IMGLOAD_FORMAT_GRAY8;
            default:
                throw std::runtime_error("Unknown data format, C++ API probably incompatible with imgloader version!");
        }
    }
}

imgload::Image::Image(ImgloadImage image, std::unique_ptr<IOHandler>&& io_handler) : m_image(image),
                                                                                     m_io(std::move(io_handler))
{
}

imgload::Image::Image(imgload::Image&& other) : m_image(nullptr), m_io(nullptr)
{
    *this = std::move(other);
}

imgload::Image::~Image()
{
    if (m_image)
    {
        imgload_image_free(m_image);
        m_image = nullptr;
    }
}

imgload::Image& imgload::Image::operator=(imgload::Image&& other)
{
    std::swap(m_image, other.m_image);
    std::swap(m_io, other.m_io);

    return *this;
}


imgload::DataFormat imgload::Image::getFormat() const
{
    switch (imgload_image_data_format(m_image))
    {
        case IMGLOAD_FORMAT_R8G8B8A8:
            return DataFormat::R8G8B8A8;
        case IMGLOAD_FORMAT_B8G8R8A8:
            return DataFormat::B8G8R8A8;
        case IMGLOAD_FORMAT_R8G8B8:
            return DataFormat::R8G8B8;
        case IMGLOAD_FORMAT_GRAY8:
            return DataFormat::GRAY8;
        default:
            throw std::runtime_error("Unknown data format, C++ API probably incompatible with imgloader version!");
    }
}

imgload::Compression imgload::Image::getCompression() const
{
    switch (imgload_image_compression(m_image))
    {
        case IMGLOAD_COMPRESSION_NONE:
            return Compression::NONE;
        case IMGLOAD_COMPRESSION_DXT1:
            return Compression::DXT1;
        case IMGLOAD_COMPRESSION_DXT2:
            return Compression::DXT2;
        case IMGLOAD_COMPRESSION_DXT3:
            return Compression::DXT3;
        case IMGLOAD_COMPRESSION_DXT4:
            return Compression::DXT4;
        case IMGLOAD_COMPRESSION_DXT5:
            return Compression::DXT5;
        default:
            throw std::runtime_error(
                    "Unknown compression format, C++ API probably incompatible with imgloader version!");
    }
}


size_t imgload::Image::numSubimages() const
{
    return imgload_image_num_subimages(m_image);
}

imgload::SubImage imgload::Image::getSubimage(size_t index) const
{
    if (index >= numSubimages())
    {
        throw std::runtime_error("Index is out of range!");
    }
    return imgload::SubImage(m_image, index);
}

void imgload::Image::readData()
{
    auto err = imgload_image_read_data(m_image);

    if (err != IMGLOAD_ERR_NO_ERROR)
    {
        throw Exception(err);
    }
}

imgload::SubImage::SubImage(ImgloadImage image, size_t i) : m_image(image), m_index(i)
{
}


size_t imgload::SubImage::numMipmaps()
{
    return imgload_image_num_mipmaps(m_image, m_index);
}

void imgload::SubImage::getPropertyInternal(imgload::Property prop, imgload::SubImage::Type type, void* val_out)
{
    ImgloadPropertyType img_type;
    switch (type)
    {
        case Type::Int32:
            img_type = IMGLOAD_PROPERTY_TYPE_INT32;
            break;
        case Type::Uint32:
            img_type = IMGLOAD_PROPERTY_TYPE_UINT32;
            break;
        case Type::Float:
            img_type = IMGLOAD_PROPERTY_TYPE_FLAOT;
            break;
        case Type::Double:
            img_type = IMGLOAD_PROPERTY_TYPE_DOUBLE;
            break;
        case Type::String:
            img_type = IMGLOAD_PROPERTY_TYPE_STRING;
            break;
        case Type::Complex:
            img_type = IMGLOAD_PROPERTY_TYPE_COMPLEX;
            break;
    }

    auto err = imgload_image_get_property(m_image, m_index, convertProperty(prop), img_type, val_out);
    if (err != IMGLOAD_ERR_NO_ERROR)
    {
        throw Exception(err);
    }
}


ImgloadImageData imgload::SubImage::getCompressedData(size_t mipmap)
{
    if (mipmap >= numMipmaps())
    {
        throw std::runtime_error("Mipmap out of range!");
    }

    ImgloadImageData data;

    auto err = imgload_image_compressed_data(m_image, m_index, mipmap, &data);
    if (err != IMGLOAD_ERR_NO_ERROR)
    {
        throw Exception(err);
    }

    return data;
}

ImgloadImageData imgload::SubImage::getImageData(size_t mipmap)
{
    if (mipmap >= numMipmaps())
    {
        throw std::runtime_error("Mipmap out of range!");
    }

    ImgloadImageData data;

    auto err = imgload_image_data(m_image, m_index, mipmap, &data);
    if (err != IMGLOAD_ERR_NO_ERROR)
    {
        throw Exception(err);
    }

    return data;
}

void Image::convertFormat(DataFormat requested, uint64_t param)
{
    auto err = imgload_image_transform_data(m_image, imgloadFormat(requested), param);

    if (err != IMGLOAD_ERR_NO_ERROR)
    {
        throw Exception(err);
    }
}

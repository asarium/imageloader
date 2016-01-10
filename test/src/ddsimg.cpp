
#include <imageloader.h>

#include <gtest/gtest.h>

#include "util.h"

#include <cstdio>

class DDSTests : public util::ContextFixture
{
};

TEST_F(DDSTests, read_header)
{
    ImgloadImage img;
    auto io = util::get_std_io();

    auto file_ptr = std::fopen(TEST_DATA_PATH "ddsimg/Col_Viper_Mk7e_Th11.dds", "rb");

    ASSERT_EQ(IMGLOAD_ERR_NO_ERROR, imgload_image_init(this->ctx, &img, &io, static_cast<void*>(file_ptr)));

    ASSERT_EQ(1, imgload_image_num_subimages(img));

    uint32_t val;
    ASSERT_EQ(IMGLOAD_ERR_NO_ERROR, imgload_image_get_property(img, 0, IMGLOAD_PROPERTY_WIDTH, IMGLOAD_PROPERTY_TYPE_UINT32, &val));
    ASSERT_EQ(512, val);

    ASSERT_EQ(IMGLOAD_ERR_NO_ERROR, imgload_image_get_property(img, 0, IMGLOAD_PROPERTY_HEIGHT, IMGLOAD_PROPERTY_TYPE_UINT32, &val));
    ASSERT_EQ(512, val);

    ASSERT_EQ(IMGLOAD_ERR_NO_ERROR, imgload_image_get_property(img, 0, IMGLOAD_PROPERTY_DEPTH, IMGLOAD_PROPERTY_TYPE_UINT32, &val));
    ASSERT_EQ(1, val);

    ASSERT_EQ(IMGLOAD_FORMAT_R8G8B8A8, imgload_image_data_format(img));
    ASSERT_EQ(IMGLOAD_COMPRESSION_DXT5, imgload_image_compression(img));

    ASSERT_EQ(7, imgload_image_num_mipmaps(img, 0));

    ASSERT_EQ(IMGLOAD_ERR_NO_ERROR, imgload_image_free(img));

    std::fclose(file_ptr);
}

TEST_F(DDSTests, read_data)
{
    ImgloadImage img;
    auto io = util::get_std_io();

    auto file_ptr = std::fopen(TEST_DATA_PATH "ddsimg/Col_Viper_Mk7e_Th11.dds", "rb");

    ASSERT_EQ(IMGLOAD_ERR_NO_ERROR, imgload_image_init(this->ctx, &img, &io, static_cast<void*>(file_ptr)));

    ASSERT_EQ(IMGLOAD_ERR_NO_ERROR, imgload_image_read_data(img));

    auto subimages = imgload_image_num_subimages(img);
    for (size_t i = 0; i < subimages; ++i)
    {
        auto mipmaps = imgload_image_num_mipmaps(img, i);
        for (size_t j = 0; j < mipmaps; ++j)
        {
            ImgloadImageData data;
            ASSERT_EQ(IMGLOAD_ERR_NO_ERROR, imgload_image_compressed_data(img, i, j, &data));

            ASSERT_EQ(IMGLOAD_ERR_NO_ERROR, imgload_image_data(img, i, j, &data));
        }
    }

    ASSERT_EQ(IMGLOAD_ERR_NO_ERROR, imgload_image_free(img));

    std::fclose(file_ptr);
}
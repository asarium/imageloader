
#include <imageloader.h>

#include <gtest/gtest.h>

#include "util.h"

#include <cstdio>

class PNGTests : public util::ContextFixture
{
};

TEST_F(PNGTests, read_header)
{
    ImgloadImage img;
    auto io = util::get_std_io();

    auto file_ptr = std::fopen(TEST_DATA_PATH "png/test1.png", "rb");

    ASSERT_EQ(IMGLOAD_ERR_NO_ERROR, imgload_image_init(this->ctx, &img, &io, static_cast<void*>(file_ptr)));

    ASSERT_EQ(1, imgload_image_num_subimages(img));
    ASSERT_EQ(1, imgload_image_num_mipmaps(img, 0));

    ASSERT_EQ(IMGLOAD_FORMAT_R8G8B8A8, imgload_image_data_format(img));
    ASSERT_EQ(IMGLOAD_COMPRESSION_NONE, imgload_image_compression(img));

    uint32_t val;
    ASSERT_EQ(IMGLOAD_ERR_NO_ERROR, imgload_image_get_property(img, 0, IMGLOAD_PROPERTY_WIDTH, IMGLOAD_PROPERTY_TYPE_UINT32, &val));
    ASSERT_EQ(800, val);

    ASSERT_EQ(IMGLOAD_ERR_NO_ERROR, imgload_image_get_property(img, 0, IMGLOAD_PROPERTY_HEIGHT, IMGLOAD_PROPERTY_TYPE_UINT32, &val));
    ASSERT_EQ(600, val);

    ASSERT_EQ(IMGLOAD_ERR_NO_ERROR, imgload_image_get_property(img, 0, IMGLOAD_PROPERTY_DEPTH, IMGLOAD_PROPERTY_TYPE_UINT32, &val));
    ASSERT_EQ(1, val);

    ASSERT_EQ(IMGLOAD_ERR_NO_ERROR, imgload_image_free(img));

    std::fclose(file_ptr);
}

TEST_F(PNGTests, read_data)
{
    ImgloadImage img;
    auto io = util::get_std_io();

    auto file_ptr = std::fopen(TEST_DATA_PATH "png/test1.png", "rb");

    ASSERT_EQ(IMGLOAD_ERR_NO_ERROR, imgload_image_init(this->ctx, &img, &io, static_cast<void*>(file_ptr)));

    ASSERT_EQ(IMGLOAD_ERR_NO_ERROR, imgload_image_read_data(img));

    ImgloadImageData data;
    ASSERT_EQ(IMGLOAD_ERR_NO_DATA, imgload_image_compressed_data(img, 0, 0, &data));

    ASSERT_EQ(IMGLOAD_ERR_NO_ERROR, imgload_image_data(img, 0, 0, &data));

    ASSERT_EQ(IMGLOAD_ERR_NO_ERROR, imgload_image_free(img));

    std::fclose(file_ptr);
}

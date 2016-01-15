
#include <imageloader.h>

#include <gtest/gtest.h>

#include "util.h"

#include <cstdio>

class STBITests : public util::ContextFixture
{
};

TEST_F(STBITests, read_header_jpeg)
{
    ImgloadImage img;
    auto io = util::get_std_io();

    auto file_ptr = std::fopen(TEST_DATA_PATH "stb_image/jpeg420exif.jpg", "rb");

    ASSERT_EQ(IMGLOAD_ERR_NO_ERROR, imgload_image_init(this->ctx, &img, &io, static_cast<void*>(file_ptr)));

    ASSERT_EQ(1, imgload_image_num_subimages(img));
    ASSERT_EQ(1, imgload_image_num_mipmaps(img, 0));

    ASSERT_EQ(IMGLOAD_FORMAT_R8G8B8, imgload_image_data_format(img));
    ASSERT_EQ(IMGLOAD_COMPRESSION_NONE, imgload_image_compression(img));

    uint32_t val;
    ASSERT_EQ(IMGLOAD_ERR_NO_ERROR, imgload_image_get_property(img, 0, IMGLOAD_PROPERTY_WIDTH, IMGLOAD_PROPERTY_TYPE_UINT32, &val));
    ASSERT_EQ(2048, val);

    ASSERT_EQ(IMGLOAD_ERR_NO_ERROR, imgload_image_get_property(img, 0, IMGLOAD_PROPERTY_HEIGHT, IMGLOAD_PROPERTY_TYPE_UINT32, &val));
    ASSERT_EQ(1536, val);

    ASSERT_EQ(IMGLOAD_ERR_NO_ERROR, imgload_image_get_property(img, 0, IMGLOAD_PROPERTY_DEPTH, IMGLOAD_PROPERTY_TYPE_UINT32, &val));
    ASSERT_EQ(1, val);

    ASSERT_EQ(IMGLOAD_ERR_NO_ERROR, imgload_image_free(img));

    std::fclose(file_ptr);
}

TEST_F(STBITests, read_data_jpeg)
{
    ImgloadImage img;
    auto io = util::get_std_io();

    auto file_ptr = std::fopen(TEST_DATA_PATH "stb_image/jpeg420exif.jpg", "rb");

    ASSERT_EQ(IMGLOAD_ERR_NO_ERROR, imgload_image_init(this->ctx, &img, &io, static_cast<void*>(file_ptr)));

    ASSERT_EQ(IMGLOAD_ERR_NO_ERROR, imgload_image_read_data(img));

    ImgloadImageData data;
    ASSERT_EQ(IMGLOAD_ERR_NO_DATA, imgload_image_compressed_data(img, 0, 0, &data));

    ASSERT_EQ(IMGLOAD_ERR_NO_ERROR, imgload_image_data(img, 0, 0, &data));

    ASSERT_EQ(IMGLOAD_ERR_NO_ERROR, imgload_image_free(img));

    std::fclose(file_ptr);
}

TEST_F(STBITests, read_data_jpeg_flip)
{
    this->makeContext(IMGLOAD_CONTEXT_FLIP_IMAGES);

    ImgloadImage img;
    auto io = util::get_std_io();

    auto file_ptr = std::fopen(TEST_DATA_PATH "stb_image/jpeg420exif.jpg", "rb");

    ASSERT_EQ(IMGLOAD_ERR_NO_ERROR, imgload_image_init(this->ctx, &img, &io, static_cast<void*>(file_ptr)));

    ASSERT_EQ(IMGLOAD_ERR_NO_ERROR, imgload_image_read_data(img));

    ImgloadImageData data;
    ASSERT_EQ(IMGLOAD_ERR_NO_DATA, imgload_image_compressed_data(img, 0, 0, &data));

    ASSERT_EQ(IMGLOAD_ERR_NO_ERROR, imgload_image_data(img, 0, 0, &data));

    ASSERT_EQ(IMGLOAD_ERR_NO_ERROR, imgload_image_free(img));

    std::fclose(file_ptr);
}

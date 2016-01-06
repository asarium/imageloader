#include <stdio.h>

#include <gtest/gtest.h>
#include <gtest/gtest-param-test.h>

#include <ddsimg/ddsimg.h>

#include "ContextFixture.h"
#include "test_data.h"

namespace
{
    size_t DDSIMG_CALLBACK file_read(void* ud, uint8_t* buf, size_t buf_len)
    {
        return fread(buf, 1, buf_len, reinterpret_cast<FILE*>(ud));
    }

    int64_t DDSIMG_CALLBACK file_seek(void* ud, int64_t offset, int whence)
    {
        if (fseek(reinterpret_cast<FILE*>(ud), offset, whence) != 0)
        {
            // Error
            return -1;
        }

        return (int64_t) ftell(reinterpret_cast<FILE*>(ud));
    }
}

class ImageTests : public ContextFixture, public ::testing::WithParamInterface<TestCaseData*>
{
};

TEST_P(ImageTests, alloc)
{
    DDSImage* img;

    // Check the api usage validation
    ASSERT_EQ(DDSIMG_ERR_INVALID_ARGUMENT, ddsimg_image_alloc(NULL, NULL, NULL, NULL));
    ASSERT_EQ(DDSIMG_ERR_INVALID_ARGUMENT, ddsimg_image_alloc(ctx, NULL, NULL, NULL));
    ASSERT_EQ(DDSIMG_ERR_INVALID_ARGUMENT, ddsimg_image_alloc(ctx, &img, NULL, NULL));
}

TEST_P(ImageTests, read_header)
{
    DDSImage* img;
    DDSIOFunctions functions;
    functions.read = file_read;
    functions.seek = file_seek;

    TestCaseData* data = GetParam();

    FILE* file = fopen(data->file_path, "rb");

    ASSERT_EQ(DDSIMG_ERR_NO_ERROR, ddsimg_image_alloc(ctx, &img, &functions, file));

    ASSERT_EQ(DDSIMG_ERR_NO_ERROR, ddsimg_image_read_header(img));

    uint32_t width;
    uint32_t height;
    uint32_t depth;
    ASSERT_EQ(DDSIMG_ERR_NO_ERROR, ddsimg_image_get_size(img, &width, &height, &depth));

    ASSERT_EQ(width, data->header.width);
    ASSERT_EQ(height, data->header.height);
    ASSERT_EQ(depth, data->header.depth);

    uint32_t compression;
    ASSERT_EQ(DDSIMG_ERR_NO_ERROR, ddsimg_image_get_compression(img, &compression));
    ASSERT_EQ(data->header.compression, compression);

    uint32_t format;
    ASSERT_EQ(DDSIMG_ERR_NO_ERROR, ddsimg_image_get_format(img, &format));
    ASSERT_EQ(format, data->header.format);

    uint32_t flags;
    ASSERT_EQ(DDSIMG_ERR_NO_ERROR, ddsimg_image_get_flags(img, &flags));
    ASSERT_EQ(flags, data->header.flags);

    uint32_t subimages;
    ASSERT_EQ(DDSIMG_ERR_NO_ERROR, ddsimg_image_get_num_subimages(img, &subimages));
    ASSERT_EQ(subimages, data->header.subimages);

    uint32_t mipmaps;
    ASSERT_EQ(DDSIMG_ERR_NO_ERROR, ddsimg_image_get_num_mipmaps(img, &mipmaps));
    ASSERT_EQ(mipmaps, data->header.mipmaps);

    ASSERT_EQ(DDSIMG_ERR_NO_ERROR, ddsimg_image_free(&img));

    fclose(file);
}

TEST_P(ImageTests, read_data)
{
    DDSImage* img;
    DDSIOFunctions functions;
    functions.read = file_read;
    functions.seek = file_seek;

    TestCaseData* data = GetParam();

    FILE* file = fopen(data->file_path, "rb");

    ASSERT_EQ(DDSIMG_ERR_NO_ERROR, ddsimg_image_alloc(ctx, &img, &functions, file));

    // Check if invalid usage is detected
    ASSERT_EQ(DDSIMG_ERR_INVALID_API_USAGE, ddsimg_image_read_data(img));

    ASSERT_EQ(DDSIMG_ERR_NO_ERROR, ddsimg_image_read_header(img));

    ASSERT_EQ(DDSIMG_ERR_NO_ERROR, ddsimg_image_read_data(img));

    ASSERT_EQ(DDSIMG_ERR_NO_ERROR, ddsimg_image_free(&img));

    fclose(file);
}

TEST_P(ImageTests, get_compressed_data)
{
    DDSImage* img;
    DDSIOFunctions functions;
    functions.read = file_read;
    functions.seek = file_seek;

    TestCaseData* data = GetParam();

    FILE* file = fopen(data->file_path, "rb");

    ASSERT_EQ(DDSIMG_ERR_NO_ERROR, ddsimg_image_alloc(ctx, &img, &functions, file));

    // Check invalid usage
    ASSERT_EQ(DDSIMG_ERR_INVALID_API_USAGE, ddsimg_image_get_compressed_data(img, 0, 0, NULL));

    ASSERT_EQ(DDSIMG_ERR_NO_ERROR, ddsimg_image_read_header(img));

    // Check invalid usage
    ASSERT_EQ(DDSIMG_ERR_INVALID_API_USAGE, ddsimg_image_get_compressed_data(img, 0, 0, NULL));

    ASSERT_EQ(DDSIMG_ERR_NO_ERROR, ddsimg_image_read_data(img));

    // Check invalid usage
    ASSERT_EQ(DDSIMG_ERR_INVALID_ARGUMENT,
              ddsimg_image_get_compressed_data(img, 0, data->header.mipmaps + 1, NULL));

    // Check invalid usage
    ASSERT_EQ(DDSIMG_ERR_INVALID_ARGUMENT,
              ddsimg_image_get_compressed_data(img, data->header.subimages + 1, 0, NULL));

    MipmapData mipmap;

    for (uint32_t subimage = 0; subimage < data->header.subimages; ++subimage)
    {
        for (uint32_t i = 0; i < data->header.mipmaps; ++i)
        {
            // Now check if the individual mipmaps were loaded correctly
            DDSErrorCode err = ddsimg_image_get_compressed_data(img, subimage, i, &mipmap);

            ASSERT_EQ(data->subimages[subimage].compressed[i].expected_return, err);

            if (err != DDSIMG_ERR_NO_ERROR)
            {
                continue;
            }

            ASSERT_EQ(data->subimages[subimage].compressed[i].width, mipmap.width);
            ASSERT_EQ(data->subimages[subimage].compressed[i].height, mipmap.height);
            ASSERT_EQ(data->subimages[subimage].compressed[i].depth, mipmap.depth);

            ASSERT_EQ(data->subimages[subimage].compressed[i].data_size, mipmap.data_size);
            ASSERT_TRUE(mipmap.data != NULL);
        }
    }

    ASSERT_EQ(DDSIMG_ERR_NO_ERROR, ddsimg_image_free(&img));

    fclose(file);
}

TEST_P(ImageTests, get_decompressed_data)
{
    DDSImage* img;
    DDSIOFunctions functions;
    functions.read = file_read;
    functions.seek = file_seek;

    TestCaseData* data = GetParam();

    FILE* file = fopen(data->file_path, "rb");

    ASSERT_EQ(DDSIMG_ERR_NO_ERROR, ddsimg_image_alloc(ctx, &img, &functions, file));

    // Check invalid usage
    ASSERT_EQ(DDSIMG_ERR_INVALID_API_USAGE, ddsimg_image_get_decompressed_data(img, 0, 0, NULL));

    ASSERT_EQ(DDSIMG_ERR_NO_ERROR, ddsimg_image_read_header(img));

    // Check invalid usage
    ASSERT_EQ(DDSIMG_ERR_INVALID_API_USAGE, ddsimg_image_get_decompressed_data(img, 0, 0, NULL));

    ASSERT_EQ(DDSIMG_ERR_NO_ERROR, ddsimg_image_read_data(img));

    // Check invalid usage
    ASSERT_EQ(DDSIMG_ERR_INVALID_ARGUMENT, ddsimg_image_get_decompressed_data(img, 0, 20, NULL));

    // Check invalid usage
    ASSERT_EQ(DDSIMG_ERR_INVALID_ARGUMENT, ddsimg_image_get_decompressed_data(img, 1, 0, NULL));

    MipmapData mipmap;

    for (uint32_t subimage = 0; subimage < data->header.subimages; ++subimage)
    {
        for (uint32_t i = 0; i < data->header.mipmaps; ++i)
        {
            // Now check if the individual mipmaps were loaded correctly
            DDSErrorCode err = ddsimg_image_get_decompressed_data(img, subimage, i, &mipmap);

            ASSERT_EQ(data->subimages[subimage].decompressed[i].expected_return, err);

            if (err != DDSIMG_ERR_NO_ERROR)
            {
                continue;
            }

            ASSERT_EQ(data->subimages[subimage].decompressed[i].width, mipmap.width);
            ASSERT_EQ(data->subimages[subimage].decompressed[i].height, mipmap.height);
            ASSERT_EQ(data->subimages[subimage].decompressed[i].depth, mipmap.depth);

            ASSERT_EQ(data->subimages[subimage].decompressed[i].data_size, mipmap.data_size);
            ASSERT_TRUE(mipmap.data != NULL);
        }
    }

    ASSERT_EQ(ddsimg_image_free(&img), DDSIMG_ERR_NO_ERROR);

    fclose(file);
}

INSTANTIATE_TEST_CASE_P(DDS_TestCases, ImageTests, ::testing::Values(&Col_Viper, &DRADIS_Target));

#ifndef DDSIMG_TEST_FILES_H
#define DDSIMG_TEST_FILES_H
#pragma once

#include <stdint.h>
#include <stddef.h>

#include <ddsimg/ddsimg.h>

struct MipmapSizeInformation
{
    uint32_t width;
    uint32_t height;
    uint32_t depth;

    size_t data_size;

    DDSErrorCode expected_return;
};

struct SubimageInformation
{
    MipmapSizeInformation* compressed;
    MipmapSizeInformation* decompressed;
};

struct TestCaseData
{
    const char* file_path;

    struct
    {
        uint32_t width;
        uint32_t height;
        uint32_t depth;

        uint32_t compression;
        uint32_t format;

        uint32_t flags;

        uint32_t subimages;
        uint32_t mipmaps;
    } header;

    SubimageInformation* subimages;
};

extern TestCaseData Col_Viper;

extern TestCaseData DRADIS_Target;

#endif //DDSIMG_TEST_FILES_H

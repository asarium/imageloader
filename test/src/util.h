
#pragma once

#include <gtest/gtest.h>

#include <imageloader.h>

namespace util
{
    class ContextFixture : public ::testing::Test
    {
    protected:
        ImgloadContext ctx;

        ContextFixture();

        void SetUp();
        void TearDown();

        void makeContext(ImgloadContextFlags flags);
    };

    ImgloadIO get_std_io();
}

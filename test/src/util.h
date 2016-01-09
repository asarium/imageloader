
#pragma once

#include <gtest/gtest.h>

#include <imageloader.h>

namespace util
{
    class ContextFixture : public ::testing::Test
    {
    protected:
        ImgloadContext ctx;

        void SetUp();
        void TearDown();
    };

    ImgloadIO get_std_io();
}

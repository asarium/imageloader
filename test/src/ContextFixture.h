#ifndef DDSIMG_CONTEXTFIXTURE_H
#define DDSIMG_CONTEXTFIXTURE_H
#pragma once

#include <gtest/gtest.h>

#include <ddsimg/ddsimg.h>

class ContextFixture : public ::testing::Test
{
protected:
    DDSContext* ctx;

    void SetUp();
    void TearDown();
};


#endif //DDSIMG_CONTEXTFIXTURE_H

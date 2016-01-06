#include "ContextFixture.h"

void ContextFixture::SetUp()
{
    ddsimg_context_alloc(&ctx, NULL, NULL);
}

void ContextFixture::TearDown()
{
    ddsimg_context_free(&ctx);
}

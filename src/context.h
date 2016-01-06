#ifndef PROJECT_CONTEXT_H
#define PROJECT_CONTEXT_H
#pragma once

#include <ddsimg/ddsimg.h>

struct DDSContext
{
    struct {
        DDSMemoryFunctions funcs;
        void* ud;
    } mem;
};

#endif //PROJECT_CONTEXT_H

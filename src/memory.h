#ifndef PROJECT_MEMORY_H
#define PROJECT_MEMORY_H
#pragma once

#include <ddsimg/ddsimg.h>

void* ddsimg_realloc(DDSContext* ctx, void* ptr, size_t size);

void ddsimg_free(DDSContext* ctx, void* ptr);

DDSMemoryFunctions* ddsimg_mem_default_memfuncs();

#endif //PROJECT_MEMORY_H

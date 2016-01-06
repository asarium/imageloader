#ifndef IMAGELOADER_MEMORY_H
#define IMAGELOADER_MEMORY_H
#pragma once

void* mem_realloc(ImgloadContext ctx, void* ptr, size_t size);

void mem_free(ImgloadContext ctx, void* ptr);

#endif //IMAGELOADER_MEMORY_H

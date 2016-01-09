#ifndef IMAGELOADER_MEMORY_H
#define IMAGELOADER_MEMORY_H
#pragma once

void* mem_realloc(ImgloadContext ctx, void* ptr, size_t size);

void* mem_reallocz(ImgloadContext ctx, void* ptr, size_t size);

void mem_free(ImgloadContext ctx, void* ptr);

char* mem_strdup(ImgloadContext ctx, const char* str);

#endif //IMAGELOADER_MEMORY_H

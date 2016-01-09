#ifndef IMAGELOADER_LOG_H
#define IMAGELOADER_LOG_H
#pragma once

#include <imageloader.h>

void print_to_log(ImgloadContext ctx, ImgloadLogLevel level, const char* format, ...);

#endif

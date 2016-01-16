#pragma once

#include "image.h"

ImgloadErrorCode format_change(ImgloadImage img, ImgloadFormat current, ImgloadImageData* data, ImgloadImageData* converted_out);

size_t format_bpp(ImgloadFormat format);

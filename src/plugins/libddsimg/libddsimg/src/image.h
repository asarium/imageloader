#ifndef DDSIMG_IMAGE_H
#define DDSIMG_IMAGE_H
#pragma once

#include <ddsimg/ddsimg.h>

#define DDSIMG_MAKEFOURCC(ch0, ch1, ch2, ch3) \
			((uint32_t)(uint8_t)(ch0) | ((uint32_t)(uint8_t)(ch1) << 8) |	\
			((uint32_t)(uint8_t)(ch2) << 16) | ((uint32_t)(uint8_t)(ch3) << 24 ))

#define DDS_CAPS				0x00000001L
#define DDS_HEIGHT				0x00000002L
#define DDS_WIDTH				0x00000004L

#define DDS_RGB					0x00000040L
#define DDS_PIXELFORMAT			0x00001000L

#define DDS_LUMINANCE			0x00020000L

#define DDS_ALPHAPIXELS			0x00000001L
#define DDS_ALPHA				0x00000002L
#define DDS_FOURCC				0x00000004L
#define DDS_PITCH				0x00000008L
#define DDS_COMPLEX				0x00000008L
#define DDS_TEXTURE				0x00001000L
#define DDS_MIPMAPCOUNT			0x00020000L
#define DDS_LINEARSIZE			0x00080000L
#define DDS_VOLUME				0x00200000L
#define DDS_MIPMAP				0x00400000L
#define DDS_DEPTH				0x00800000L

#define DDS2_CUBEMAP			0x00000200L
#define DDS2_CUBEMAP_POSITIVEX	0x00000400L
#define DDS2_CUBEMAP_NEGATIVEX	0x00000800L
#define DDS2_CUBEMAP_POSITIVEY	0x00001000L
#define DDS2_CUBEMAP_NEGATIVEY	0x00002000L
#define DDS2_CUBEMAP_POSITIVEZ	0x00004000L
#define DDS2_CUBEMAP_NEGATIVEZ	0x00008000L

typedef struct DDS_PIXELFMT
{
    uint32_t dwSize;
    uint32_t dwFlags;
    uint32_t dwFourCC;
    uint32_t dwRGBBitCount;
    uint32_t dwRBitMask;
    uint32_t dwGBitMask;
    uint32_t dwBBitMask;
    uint32_t dwABitMask;
} DDS_PIXELFMT;

typedef struct DDS_HEADER
{
    uint32_t dwSize;
    uint32_t dwFlags;
    uint32_t dwHeight;
    uint32_t dwWidth;
    uint32_t dwPitchOrLinearSize;
    uint32_t dwDepth;
    uint32_t dwMipMapCount;
    uint32_t dwReserved1[11];
    DDS_PIXELFMT ddspf;
    uint32_t dwCaps;
    uint32_t dwCaps2;
    uint32_t dwCaps3;
    uint32_t dwCaps4;
    uint32_t dwReserved2;
} DDS_HEADER;

typedef struct
{
    MipmapData* compressed;

    MipmapData* decompressed;
} subimage_t;

struct DDSImage
{
    DDSContext* ctx;

    struct
    {
        DDSIOFunctions funcs;
        void* ud;
    } io;

    DDS_HEADER header;

    int header_parsed;
    int data_read;

    struct
    {
        DDSCompressedFormat_t compression_format;
        DDSFormat_t decompressed_format;
        uint32_t subimages;
        uint32_t flags;
    } props;

    subimage_t* subimages;
};

size_t ddsimg_read(DDSImage* img, uint8_t* buf, size_t buf_len);

int64_t ddsimg_seek(DDSImage* img, int64_t offset, int whence);

#endif //DDSIMG_IMAGE_H

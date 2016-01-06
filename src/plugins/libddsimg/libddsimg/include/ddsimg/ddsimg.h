#ifndef DDSIMG_DDSIMG_H
#define DDSIMG_DDSIMG_H
#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include <stddef.h>

#include <ddsimg/ddsimg_export.h>

// Standard calling convention
#ifdef WIN32
#define DDSIMG_CC __stdcall
#else
#define DDSIMG_CC
#endif

#define DDSIMG_API DDSIMG_EXPORT DDSIMG_CC
#define DDSIMG_CALLBACK DDSIMG_CC

/**
 * @brief The major version of the library this header belongs to
 */
#define DDSIMG_VERSION_MAJOR 0
/**
 * @brief The minor version of the library this header belongs to
 */
#define DDSIMG_VERSION_MINOR 1
/**
 * @brief The patch version of the library this header belongs to
 */
#define DDSIMG_VERSION_PATCH 0

/**
 * @brief Determines the major version this library was compiled with.
 * @note  Compare with DDSIMG_VERSION_MAJOR to check if the runtime version is compatible
 * @return The major version of the runtime library
 */
uint32_t DDSIMG_API ddsimg_version_major();

/**
 * @brief Determines the minor version this library was compiled with.
 * @note  Compare with DDSIMG_VERSION_MINOR to check if the runtime version is compatible
 * @return The minor version of the runtime library
 */
uint32_t DDSIMG_API ddsimg_version_minor();

/**
 * @brief Determines the patch version this library was compiled with.
 * @note  Compare with DDSIMG_VERSION_PATCH to check if the runtime version is compatible
 * @return The patch version of the runtime library
 */
uint32_t DDSIMG_API ddsimg_version_patch();

/**
 * @brief Defines the various types of error codes an API function can return
 */
typedef enum
{
    DDSIMG_ERR_NO_ERROR = 0,         //!< No error has happened
    DDSIMG_ERR_INVALID_API_USAGE = 1, //!< The called function may not be used like that
    DDSIMG_ERR_OUT_OF_MEMORY = 2,    //!< Trying to allocate new memory failed
    DDSIMG_ERR_FILE_INVALID = 3,     //!< The file is not a valid DDS file
    DDSIMG_ERR_FORMAT_UNKNOWN = 4,   //!< The format of the file is unknown or not supported
    DDSIMG_ERR_NOT_SUPPORTED = 5,    //!< The operation is not supported
    DDSIMG_ERR_FILE_SIZE_WRONG = 6,  //!< The library determined that the file is not valid because the file size is wrong
    DDSIMG_ERR_INVALID_ARGUMENT = 7, //!< A passed argument is not valid
    DDSIMG_ERR_NO_DATA = 8,          //!< There is no data for the requested mipmap (e.g. when cubemaps lacks the wanted face)
} DDSErrorCodes_t;

/**
 * @brief The various compressed formats
 */
typedef enum
{
    DDSIMG_COM_DXT1 = 0,      //!< DDSIMG_COM_DXT1
    DDSIMG_COM_DXT2 = 1,      //!< DDSIMG_COM_DXT2
    DDSIMG_COM_DXT3 = 2,      //!< DDSIMG_COM_DXT3
    DDSIMG_COM_DXT4 = 3,      //!< DDSIMG_COM_DXT4
    DDSIMG_COM_DXT5 = 4,      //!< DDSIMG_COM_DXT5
    DDSIMG_COM_UNKNOWN = 0xFF,//!< DDSIMG_COM_UNKNOWN
} DDSCompressedFormat_t;

/**
 * @brief  The various raw pixel formats
 */
typedef enum
{
    DDSIMG_FORMAT_R8G8B8A8 = 0,  //!< DDSIMG_FORMAT_R8G8B8A8
    DDSIMG_FORMAT_UNKNOWN = 0xFF,//!< DDSIMG_FORMAT_UNKNOWN
} DDSFormat_t;

/**
 * @brief Flags for an image
 */
typedef enum
{
    DDSIMG_IMG_CUBEMAP = 1 << 0//!< This image is a cubemap
} DDSImageFlags_t;

/**
 * @brief Return type for a function that can fail in some way
 */
typedef uint32_t DDSErrorCode;

/**
 * @brief A context for DDS operations
 * This is currently used for memory allocations but may be extended in the future
 */
typedef struct DDSContext DDSContext;

/**
 * @brief A DDS file
 */
typedef struct DDSImage DDSImage;

/**
 * @brief Custom memory allocation functions
 * Should be used if a custom memory allocator should be used in the library.
 *
 * @warning Thread-safety and reentrancy guarantees only hold if these functions are Thread-safe!
 */
typedef struct
{
    /**
     * @brief Should behave like the standard realloc
     * @param ud The userdata passed at context creation
     * @param mem The memory to be reallocated (or @c NULL)
     * @param size The wanted new size
     * @return The new memory or @c NULL on failure
     */
	void* (DDSIMG_CALLBACK *realloc)(void* ud, void* mem, size_t size);

    /**
     * @brief Should behave like the standard free
     * @param ud The userdata passed at context creation
     * @param mem The memory to be freed
     */
    void (DDSIMG_CALLBACK *free)(void* ud, void* mem);
} DDSMemoryFunctions;

/**
 * @brief Custom IO operations callbacks
 */
typedef struct
{
    /**
     * @brief Read data into a buffer
     * @param ud The userdata passed at image allocation
     * @param buf The buffer to write data into
     * @param size The size to read into the buffer
     * @return The number of bytes read, return 0 at end of stream
     */
    size_t (DDSIMG_CALLBACK *read)(void* ud, uint8_t* buf, size_t size);

    /**
     * @brief Seek within the file
     * @param ud The userdata passed at image allocation
     * @param offset The offset to seek to in the file
     * @param whence The seek mode using the stdio defines
     * @return The offset in the file after seeking
     */
    int64_t (DDSIMG_CALLBACK *seek)(void* ud, int64_t offset, int whence);
} DDSIOFunctions;

typedef struct
{
    uint32_t width;
    uint32_t height;
    uint32_t depth;

    size_t data_size;
    void* data;
} MipmapData;

/**
 * @brief Allocates a context
 * @param [out] contextPtr The variable where the context should be stored
 * @param mem_funcs The memory allocation functions that should be used, may be NULL to use standard @c realloc and @c free
 * @param mem_ud The userdata for the memory allocation functions
 * @return The error code
 *
 * @note This function is thread-safe
 */
DDSErrorCode DDSIMG_API ddsimg_context_alloc(DDSContext** contextPtr, DDSMemoryFunctions* mem_funcs, void* mem_ud);

/**
 * @brief Frees memory of the given context
 * @warning Images allocated from this context must be freed before this context!
 * @param ctx [in,out] The context to free
 * @return The error code
 *
 * @note This function is reentrant
 */
DDSErrorCode DDSIMG_API ddsimg_context_free(DDSContext** ctx);

/**
 * @brief Allocates an image structure
 * This only allocates and initializes the memory for the structure but doesn't read anything.
 * @param context The context which should be used
 * @param [out] image The image pointer
 * @param io_funcs The custom IO functions to use
 * @param io_ud The userdata for the IO functions
 * @return The error code
 */
DDSErrorCode DDSIMG_API ddsimg_image_alloc(DDSContext* context, DDSImage** image, DDSIOFunctions* io_funcs,
                                           void* io_ud);

/**
 * @brief Reads the header of the file
 * @param image The image to read the header of
 * @return The error code
 */
DDSErrorCode DDSIMG_API ddsimg_image_read_header(DDSImage* image);

/**
 * @brief Gets the size of the image
 * @param image The image to get the size of
 * @param [out] width_out The width
 * @param [out] height_out The height
 * @param [out] depth_out The depth
 * @return The error code
 */
DDSErrorCode DDSIMG_API ddsimg_image_get_size(DDSImage* image, uint32_t* width_out, uint32_t* height_out,
                                              uint32_t* depth_out);

/**
 * @brief Gets image flags, see DDSImageFlags_t for values
 * @param image The image to use
 * @param [out] flags_out The image flags
 * @return The error code
 */
DDSErrorCode DDSIMG_API ddsimg_image_get_flags(DDSImage* image, uint32_t* flags_out);

/**
 * @brief Gets the compression format of the image
 * @param image The image to use
 * @param [out] comp_out The compression format
 * @return The error code
 */
DDSErrorCode DDSIMG_API ddsimg_image_get_compression(DDSImage* image, uint32_t* comp_out);

/**
 * @brief Gets the pixel format of the image
 * @param image The image
 * @param [out] format_out The format
 * @return The error code
 */
DDSErrorCode DDSIMG_API ddsimg_image_get_format(DDSImage* image, uint32_t* format_out);

/**
 * @brief Gets the number of mipmaps in the image
 * @param image The image
 * @param [out] num_mipmaps_out The number of mipmaps
 * @return The error code
 */
DDSErrorCode DDSIMG_API ddsimg_image_get_num_mipmaps(DDSImage* image, uint32_t* num_mipmaps_out);

/**
 * @brief Gets the number of subimages in this image
 * @param image The image
 * @param [out] num_subimages_out The number of subimages
 * @return The error code
 */
DDSErrorCode DDSIMG_API ddsimg_image_get_num_subimages(DDSImage* image, uint32_t* num_subimages_out);

/**
 * @brief Reads the image data of the image
 * @param image The image to read data for
 * @note You must call #ddsimg_image_read_header before calling this function
 * @return The error code
 */
DDSErrorCode DDSIMG_API ddsimg_image_read_data(DDSImage* image);

/**
 * @brief Gets the compressed data of a specific mipmap
 * @param image The image to use
 * @param subimage The wanted subimage
 * @param mipmap_index The wanted mipmap
 * @param [out] data The struct to write the mipmap data to
 * @note You must call #ddsimg_read_header and #ddsimg_read_data before using this function!
 * @return The error code
 */
DDSErrorCode DDSIMG_API ddsimg_image_get_compressed_data(DDSImage* image, uint32_t subimage, uint32_t mipmap_index,
                                                         MipmapData* data);

/**
 * @brief Gets the decompressed data of a specific mipmap
 * @param image The image to use
 * @param subimage The wanted subimage
 * @param mipmap_index The wanted mipmap
 * @param [out] data The struct to write the mipmap data to
 * @note You must call #ddsimg_read_header and #ddsimg_read_data before using this function!
 * @return The error code
 */
DDSErrorCode DDSIMG_API ddsimg_image_get_decompressed_data(DDSImage* image, uint32_t subimage, uint32_t mipmap_index,
                                                           MipmapData* data);

/**
 * @brief Frees all memory of an image
 * @param [in,out] image The image to free memory of
 * @return The error code
 */
DDSErrorCode DDSIMG_API ddsimg_image_free(DDSImage** image);

#ifdef __cplusplus
}
#endif

#endif //DDSIMG_DDSIMG_H

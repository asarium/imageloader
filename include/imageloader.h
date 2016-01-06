#ifndef IMAGELOADER_IMAGELOADER_H
#define IMAGELOADER_IMAGELOADER_H
#pragma once

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif

// Standard calling convention
#ifdef WIN32
#define IMGLOAD_CC __stdcall
#else
#define IMGLOAD_CC
#endif

#if defined(WIN32)
	#ifdef IMGLOAD_BUILDING_DLL
		#ifdef IMGLOAD_COMPILING
			#define IMGLOAD_EXPORT __declspec(dllexport)
		#else
			#define IMGLOAD_EXPORT __declspec(dllimport)
		#endif
	#else
		#define IMGLOAD_EXPORT
	#endif
#else
	#if defined(__GNUC__) && __GNUC__ >= 4
		#define IMGLOAD_EXPORT __attribute__ ((visibility("default")))
	#else
		#define IMGLOAD_EXPORT
	#endif
#endif

#define IMGLOAD_API IMGLOAD_EXPORT IMGLOAD_CC
#define IMGLOAD_CALLBACK IMGLOAD_CC

/**
 * @brief The major version of the library this header belongs to
 */
#define IMGLOAD_VERSION_MAJOR 0
/**
 * @brief The minor version of the library this header belongs to
 */
#define IMGLOAD_VERSION_MINOR 1
/**
 * @brief The patch version of the library this header belongs to
 */
#define IMGLOAD_VERSION_PATCH 0

/**
 * @brief Determines the major version this library was compiled with.
 * @note  Compare with IMGLOAD_VERSION_MAJOR to check if the runtime version is compatible
 * @return The major version of the runtime library
 */
uint32_t IMGLOAD_API imgload_version_major();

/**
 * @brief Determines the minor version this library was compiled with.
 * @note  Compare with IMGLOAD_VERSION_MINOR to check if the runtime version is compatible
 * @return The minor version of the runtime library
 */
uint32_t IMGLOAD_API imgload_version_minor();

/**
 * @brief Determines the patch version this library was compiled with.
 * @note  Compare with IMGLOAD_VERSION_PATCH to check if the runtime version is compatible
 * @return The patch version of the runtime library
 */
uint32_t IMGLOAD_API imgload_version_patch();


/**
 * @brief Defines the various types of error codes an API function can return
 */
enum
{
    IMGLOAD_ERR_NO_ERROR = 0,
    IMGLOAD_ERR_OUT_OF_MEMORY = 1,
    IMGLOAD_ERR_PLUGIN_INVALID = 2,
    IMGLOAD_UNSUPPORTED_FORMAT = 3,
};
typedef uint32_t ImgloadErrorCode;

enum
{
	IMGLOAD_LOG_DEBUG = 0,
	IMGLOAD_LOG_INFO = 1,
	IMGLOAD_LOG_WARNING = 2,
	IMGLOAD_LOG_ERROR = 3,
};
typedef uint32_t ImgloadLogLevel;

/**
 * @brief Custom memory allocation functions
 * Should be used if a custom memory allocator should be used in the library.
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
    void* (IMGLOAD_CALLBACK *realloc)(void* ud, void* mem, size_t size);

    /**
     * @brief Should behave like the standard free
     * @param ud The userdata passed at context creation
     * @param mem The memory to be freed
     */
    void (IMGLOAD_CALLBACK *free)(void* ud, void* mem);
} ImgloadMemoryAllocator;

typedef struct ImgloadContextImpl* ImgloadContext;

typedef struct ImgloadPluginImpl* ImgloadPlugin;

typedef ImgloadErrorCode (IMGLOAD_CALLBACK *ImgloadPluginLoader)(ImgloadPlugin plugin, void* parameter);

typedef ImgloadErrorCode(IMGLOAD_CALLBACK *ImgloadLogHandler)(void* ud, ImgloadLogLevel level, const char* text);


ImgloadErrorCode IMGLOAD_API imgload_context_alloc(ImgloadContext* ctx_ptr, ImgloadMemoryAllocator* allocator, void* alloc_ud);

const char* IMGLOAD_API imgload_context_get_error(ImgloadContext ctx);

ImgloadErrorCode IMGLOAD_API imgload_context_add_plugin(ImgloadContext ctx, ImgloadPluginLoader loader_func, void* plugin_param);

ImgloadErrorCode IMGLOAD_API imgload_context_set_log_callback(ImgloadContext ctx, ImgloadLogHandler handler, void* ud);

ImgloadErrorCode IMGLOAD_API imgload_context_free(ImgloadContext ctx);


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
    size_t (IMGLOAD_CALLBACK *read)(void* ud, uint8_t* buf, size_t size);

    /**
     * @brief Seek within the file
     * @param ud The userdata passed at image allocation
     * @param offset The offset to seek to in the file
     * @param whence The seek mode using the stdio defines
     * @return The offset in the file after seeking
     */
    int64_t (IMGLOAD_CALLBACK *seek)(void* ud, int64_t offset, int whence);
} ImgloadCustomIO;

typedef struct ImgloadImageImpl* ImgloadImage;

ImgloadErrorCode IMGLOAD_API imgload_image_alloc(ImgloadContext ctx, ImgloadImage* image, ImgloadCustomIO* io, void* io_ud);

ImgloadErrorCode IMGLOAD_API imgload_read_header(ImgloadImage img);

ImgloadErrorCode IMGLOAD_API imgload_image_free(ImgloadImage image);

#ifdef __cplusplus
}
#endif

#endif //IMAGELOADER_IMAGELOADER_H

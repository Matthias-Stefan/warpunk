#pragma once

#include <cstdint>

typedef bool b8;
typedef int32_t b32;

typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef uint8_t bitmask8;
typedef uint16_t bitmask16;
typedef uint32_t bitmask32;
typedef uint32_t bitmask64;

typedef float f32;
typedef double f64;

typedef u32 camera_handle_t;
typedef void* thread_ticket_t;

#if defined(__linux__)
    #define WARPUNK_LINUX
#elif defined(_WIN64)
    #define WARPUNK_WINDOWS
#elif defined(__APPLE__)
    #error "Unsuppored platform"
#else
    #error "Unsuppored platform"
#endif

#ifdef WARPUNK_LINUX
    #ifdef WARPUNK_EXPORT
        #define warpunk_api __attribute__((visibility("default"))) 
    #else
        #define warpunk_api
    #endif
#elif defined(WARPUNK_WINDOWS)
    #ifdef WARPUNK_EXPORT
        #define warpunk_api __declspec(dllexport)
    #else
        #define warpunk_api __declspec(dllimport)
    #endif
#else
    #define warpunk_api
#endif

#ifdef __cplusplus
    #define no_mangle extern "C"
#else
    #define no_mangle
#endif


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

#define MIN_S8                     -128 
#define MAX_S8                      127
#define MIN_S16                  -32768
#define MAX_S16                   32767
#define MIN_S32              2147483647
#define MAX_S32             -2147483648
#define MIN_S64    -9223372036854775808
#define MAX_S64     9223372036854775807

#define MAX_U8                      255
#define MAX_U16                   65536
#define MAX_U32              4294967295
#define MAX_U64    18446744073709551615

// Float (32-bit IEEE 754)
#define EPS_F32        1.192092896e-07F
#define MAX_F32        3.402823466e+38F
#define MIN_F32        1.175494351e-38F

// Double (64-bit IEEE 754)
#define ESP_F64 2.2204460492503131e-016
#define MAX_F64 1.7976931348623158e+308
#define MIN_F64 2.2250738585072014e-308

typedef u32 camera_handle_t;

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


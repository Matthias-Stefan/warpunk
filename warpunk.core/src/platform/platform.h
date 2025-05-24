#pragma once

#include "warpunk.core/src/defines.h"
#include "warpunk.core/src/input_system/input_types.h"
#include "warpunk.core/src/utils/logger.h"

#include <vector>

typedef struct function_description
{
    const char* name;
    void* function;
    void* library_handle;
    b8 is_dirty;
    f32 total_execution_time;
    f32 average_execution_time;
} function_description;

typedef struct library_context
{
    void* handle;
    std::vector<function_description*> functions;
} library_context;

typedef enum platform_window_mode
{
    FULLSCREEN,
    WINDOWED,

    PLATFORM_WINDOW_MODE_COUNT
} platform_window_mode;

typedef struct platform_window_info
{
    char* title;
    s16 width;
    s16 height;
    b8 is_visible;
    s16 monitor_index;
    f32 dpi_scale;
    platform_window_mode platform_window_mode;
} platform_window_info;

typedef u32 thread_ticket;
typedef struct platform_threading_job
{
    void(*function)(void *);
    void* arg;
    s64 arg_size;
} platform_threading_job;

/** */
no_mangle warpunk_api bool platform_startup();

/** */
no_mangle warpunk_api void platform_shutdown();

/** */
no_mangle warpunk_api void platform_process_input();

/** */
no_mangle warpunk_api b8 platform_get_window_handle(s32* size, void* platform_handle);

//* */
no_mangle warpunk_api b8 platform_load_library(const char* path, library_context* out_library_context);

//* */
no_mangle warpunk_api b8 platform_unload_library(library_context* library_context);

//* */
no_mangle warpunk_api b8 platform_get_function(library_context* library_context, function_description* out_function_description);

/** */
no_mangle warpunk_api b8 platform_is_mouse_inside_window();

/** */
no_mangle warpunk_api b8 platform_set_window_mode(platform_window_mode platform_window_mode);

/** */
no_mangle warpunk_api b8 platform_get_window_info(platform_window_info* platform_window_info);

/**
 * =================== PLATFORM MEMORY ===================
 */

/** */
no_mangle warpunk_api void* platform_memory_alloc(s64 size);

/** */
no_mangle warpunk_api void platform_memory_free(void* src);

/** */
no_mangle warpunk_api void platform_memory_copy(void* dst, void* src, s64 size);

/** */
no_mangle warpunk_api void platform_memory_set(void* dst, s64 size, s32 value);

/** */
no_mangle warpunk_api void platform_memory_zero(void* dst, s64 size);

/**
 * =================== PLATFORM CONSOLE ===================
 */

no_mangle warpunk_api void platform_console_write(log_level level, const char* message);

/**
 * =================== PLATFORM CLOCK ===================
 */

no_mangle warpunk_api f64 platform_get_absolute_time();

/**
 * =================== PLATFORM THREADING ===================
 */

/** */
no_mangle warpunk_api void platform_threadpool_add(platform_threading_job* jobs, u32 chunk_count, thread_ticket* out_ticket);

/** */
no_mangle warpunk_api void platform_threadpool_sync(thread_ticket ticket, f64 cancellation_time);

/**
 * =================== PLATFORM EVENTS ===================
 */

/** */
typedef void (*platform_keyboard_event_t)(keycode keycode, b8 pressed);
/** */
no_mangle warpunk_api void platform_register_keyboard_event(platform_keyboard_event_t callback);

/** */
typedef void (*platform_mouse_button_event_t)(mouse_button mouse_button, b8 pressed);
/** */
no_mangle warpunk_api void platform_register_mouse_button_event(platform_mouse_button_event_t callback);

/** */
typedef void (*platform_mouse_move_event_t)(s16 x, s16 y);
/** */
no_mangle warpunk_api void platform_register_mouse_move_event(platform_mouse_move_event_t callback);

/** */
typedef void (*platform_mouse_wheel_event_t)(s32 delta); 
/** */
no_mangle warpunk_api void platform_register_mouse_wheel_event(platform_mouse_wheel_event_t callback);



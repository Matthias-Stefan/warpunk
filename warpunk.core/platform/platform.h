#pragma once

#include <vector>

#include "warpunk.core/defines.h"
#include "warpunk.core/input_system/input_types.h"

//////////////////////////////////////////////////////////////////////

typedef struct _function_description_t
{
    const char* name;
    void* function;
    void* library_handle;
    b8 is_dirty;
    f32 total_execution_time;
    f32 average_execution_time;
} function_description_t;

typedef struct _library_context_t
{
    void* handle;
    std::vector<function_description_t*> functions;
} library_context_t;

typedef enum _platform_window_mode_t
{
    FULLSCREEN,
    WINDOWED,

    PLATFORM_WINDOW_MODE_COUNT
} platform_window_mode_t;

typedef struct _platform_window_info_t
{
    char* title;
    s16 width;
    s16 height;
    b8 is_visible;
    s16 monitor_index;
    f32 dpi_scale;
    platform_window_mode_t platform_window_mode;
} platform_window_info_t;

/** */
bool platform_startup();

/** */
void platform_shutdown();

/** */
void platform_process_input();   

/** */
[[nodiscard]] b8 platform_get_window_handle(s32* size, void* platform_handle);

//* */
warpunk_api [[nodiscard]] b8 platform_load_library(const char* path, library_context_t* out_library_context);

//* */
warpunk_api [[nodiscard]] b8 platform_unload_library(library_context_t* library_context);

//* */
warpunk_api [[nodiscard]] b8 platform_get_function(library_context_t* library_context, function_description_t* out_function_description);

/** */
warpunk_api [[nodiscard]] b8 platform_is_mouse_inside_window();

/** */
warpunk_api [[nodiscard]] b8 platform_set_window_mode(platform_window_mode_t platform_window_mode);

/** */
warpunk_api [[nodiscard]] b8 platform_get_window_info(platform_window_info_t* platform_window_info);

/** ------ */
/** memory */
/** ------ */

/** */
warpunk_api [[nodiscard]] void* platform_memory_alloc(s64 size);

/** */
warpunk_api void platform_memory_free(void* src);

/** */
warpunk_api void platform_memory_copy(void* dst, void* src, s64 size);

/** */
warpunk_api void platform_memory_set(void* dst, s64 size, s32 value);

/** */
warpunk_api void platform_memory_zero(void* dst, s64 size);

/** --------- */
/** threading */
/** --------- */

typedef u32 thread_ticket_t;

typedef struct _platform_threading_job_t
{
    void(*function)(void *);
    void* arg;
    s64 arg_size;
} platform_threading_job_t;

/** */
warpunk_api void 
platform_threadpool_add(platform_threading_job_t* jobs, u32 chunk_count, thread_ticket_t* out_ticket);

/** */
warpunk_api void
platform_threadpool_sync(thread_ticket_t ticket, f64 cancellation_time);

/** ------ */
/** events */
/** ------ */

/** */
typedef void (*platform_keyboard_event_t)(keycode_t keycode, b8 pressed);
/** */
void platform_register_keyboard_event(platform_keyboard_event_t callback);

/** */
typedef void (*platform_mouse_button_event_t)(mouse_button_t mouse_button, b8 pressed);
/** */
void platform_register_mouse_button_event(platform_mouse_button_event_t callback);

/** */
typedef void (*platform_mouse_move_event_t)(s16 x, s16 y);
/** */
void platform_register_mouse_move_event(platform_mouse_move_event_t callback);

/** */
typedef void (*platform_mouse_wheel_event_t)(s32 delta); 
/** */
void platform_register_mouse_wheel_event(platform_mouse_wheel_event_t callback);

// TODO: game related

//* */
typedef void (*game_startup_t)();
//* */
warpunk_api no_mangle void game_startup();

//* */
typedef void (*game_tick_t)(double);
//* */
warpunk_api no_mangle void game_tick(double dt);



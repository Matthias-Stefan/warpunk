#include "warpunk.core/src/defines.h"
#if defined(WARPUNK_WINDOWS)

#include "warpunk.core/src/utils/logger.h"
#include "warpunk.core/src/platform/platform.h"

#include <windows.h>
#include <windowsx.h>
#include <stdlib.h>
#include <timeapi.h>

static f64 clock_frequency;
static UINT min_period;
static LARGE_INTEGER start_time;

static void win32_clock_setup(void) 
{
    LARGE_INTEGER frequency;
    QueryPerformanceFrequency(&frequency);
    clock_frequency = 1.0 / (f64)frequency.QuadPart;
    QueryPerformanceCounter(&start_time);

    TIMECAPS tc;
    timeGetDevCaps(&tc, sizeof(tc));
    min_period = tc.wPeriodMin;
}

bool platform_startup()
{
    win32_clock_setup();

    return true;
}

void platform_shutdown()
{
}


no_mangle warpunk_api void platform_process_input()
{
    int x = 5;
    (void)x;
}

b8 platform_get_window_handle(s32* size, void* platform_handle)
{
    return true;
}

b8 platform_load_library(const char* path, library_context* out_library_context)
{
    return true;
}

b8 platform_unload_library(library_context* library_context)
{
    return true;
}

b8 platform_get_function(library_context* library_context, function_description* out_function_description)
{
    return true;
}

b8 platform_is_mouse_inside_window()
{
    return true;
}

b8 platform_set_window_mode(platform_window_mode platform_window_mode)
{
    return true;
}

b8 platform_get_window_info(platform_window_info* platform_window_info)
{
    return true;
}

void* platform_memory_alloc(s64 size)
{
    return nullptr;
}

void platform_memory_free(void* src)
{
}

void platform_memory_copy(void* dst, void* src, s64 size)
{
}

void platform_memory_set(void* dst, s64 size, s32 value)
{
}

void platform_memory_zero(void* dst, s64 size)
{
}

no_mangle warpunk_api void platform_console_write(log_level level, const char* message)
{
    b8 is_error = (level == LOG_LEVEL_ERROR || level == LOG_LEVEL_FATAL);
    HANDLE console_handle = GetStdHandle(is_error ? STD_ERROR_HANDLE : STD_OUTPUT_HANDLE);

    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(console_handle, &csbi);
    WORD original_attributes = csbi.wAttributes;

    static WORD levels[7] = {
        FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY, // INFO
        FOREGROUND_GREEN | FOREGROUND_INTENSITY,                   // SUCCESS
        FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY,  // WARNING
        FOREGROUND_RED | FOREGROUND_INTENSITY,                     // ERROR
        BACKGROUND_RED | FOREGROUND_INTENSITY,                     // FATAL
        FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_INTENSITY,   // DEBUG
        FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE        // VERBOSE
    };

    SetConsoleTextAttribute(console_handle, levels[level]);

    DWORD written;
    WriteConsoleA(console_handle, message, (DWORD)strlen(message), &written, NULL);

    OutputDebugStringA(message);

    SetConsoleTextAttribute(console_handle, original_attributes);
}

f64 platform_get_absolute_time()
{
    if (!clock_frequency)
    {
        win32_clock_setup();
    }

    LARGE_INTEGER now_time;
    QueryPerformanceCounter(&now_time);
    return (f64)now_time.QuadPart * clock_frequency;
}

void platform_threadpool_add(platform_threading_job* jobs, u32 chunk_count, thread_ticket* out_ticket)
{
}

void platform_threadpool_sync(thread_ticket ticket, f64 cancellation_time)
{
}

void platform_register_keyboard_event(platform_keyboard_event_t callback)
{
}

void platform_register_mouse_button_event(platform_mouse_button_event_t callback)
{
}

void platform_register_mouse_move_event(platform_mouse_move_event_t callback)
{
}

void platform_register_mouse_wheel_event(platform_mouse_wheel_event_t callback)
{
}

#endif
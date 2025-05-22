#pragma once

#include "warpunk.core/src/defines.h"

#if defined(WARPUNK_LINUX)

#include <X11/Xlib-xcb.h>

typedef struct linux_handle
{
    xcb_connection_t* connection;
    xcb_window_t window;
    xcb_screen_t* screen;
} linux_handle;

b8 platform_result_is_success(xcb_void_cookie_t cookie);

b8 platform_get_linux_handle(linux_handle* out_linux_handle);

#endif // WARPUNK_LINUX

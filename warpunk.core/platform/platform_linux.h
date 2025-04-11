#pragma once

#include "warpunk.core/defines.h"

#if defined(WARPUNK_LINUX)

#include <X11/Xlib-xcb.h>

typedef struct _linux_handle_info_t
{
    xcb_connection_t* connection;
    xcb_window_t window;
} linux_handle_info_t;

b8 platform_result_is_success(xcb_void_cookie_t cookie);

b8 platform_linux_get_connection(xcb_connection_t** out_connection);

b8 platform_linux_get_window(xcb_window_t* out_window);

#endif // WARPUNK_LINUX

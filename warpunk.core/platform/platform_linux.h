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

#endif // WARPUNK_LINUX

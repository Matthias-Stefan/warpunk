#pragma once

#include "warpunk.core/defines.h"

#include <xcb/xcb.h>
#include <xcb/xproto.h>
#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/Xlib-xcb.h>
#include <X11/XKBlib.h>
#include <X11/keysym.h>
#include <X11/keysymdef.h>

typedef struct _linux_handle_info_t
{
    xcb_connection_t* connection;
    xcb_window_t window;
} linux_handle_info_t;

[[nodiscard]] b8 platform_result_is_success(xcb_void_cookie_t cookie);


#include "platform.h"

#include <X11/X.h>
#include <cassert>
#include <cstdlib>
#include <cstdio>
#include <dlfcn.h>
#include <xcb/xcb.h>
#include <xcb/xproto.h>
#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/Xlib-xcb.h>
#include <X11/XKBlib.h>
#include <X11/keysym.h>
#include <X11/keysymdef.h>
#include <vector>

#include "input_types.h"
#include "defines.h"

//////////////////////////////////////////////////////////////////////

#define PLATFORM_MOUSE_BUTTON_LEFT XCB_BUTTON_INDEX_1
#define PLATFORM_MOUSE_BUTTON_MIDDLE XCB_BUTTON_INDEX_2
#define PLATFORM_MOUSE_BUTTON_RIGHT XCB_BUTTON_INDEX_3
#define PLATFORM_MOUSE_WHEEL_UP XCB_BUTTON_INDEX_4
#define PLATFORM_MOUSE_WHEEL_DOWN XCB_BUTTON_INDEX_5
#define PLATFORM_MOUSE_WHEEL_LEFT 6
#define PLATFORM_MOUSE_WHEEL_RIGHT 7
#define PLATFORM_MOUSE_BUTTON_1 8
#define PLATFORM_MOUSE_BUTTON_2 9
#define PLATFORM_MOUSE_BUTTON_3 10
#define PLATFORM_MOUSE_BUTTON_4 11
#define PLATFORM_MOUSE_BUTTON_5 12
#define PLATFORM_MOUSE_BUTTON_6 13
#define PLATFORM_MOUSE_BUTTON_7 14
#define PLATFORM_MOUSE_BUTTON_8 15
#define PLATFORM_MOUSE_BUTTON_9 16

[[nodiscard]] static keycode_t translate_keycode(const unsigned int key_code);

//////////////////////////////////////////////////////////////////////

typedef struct _linux_window_mode_hints_t
{
    unsigned long flags;
    unsigned long functions;
    unsigned long decorations;
    long input_mode;
    unsigned long status;
} linux_window_mode_hints_t;

typedef struct _linux_state_t
{
    Display* display;
    xcb_connection_t* connection;
    xcb_window_t window;
    xcb_screen_t* screen;
    platform_window_info_t window_info;

    platform_keyboard_event_t keyboard_event;
    platform_mouse_button_event_t mouse_button_event;
    platform_mouse_move_event_t mouse_move_event;
    platform_mouse_wheel_event_t mouse_wheel_event;
} linux_state_t;

//////////////////////////////////////////////////////////////////////

// NOTE: Global

static linux_state_t linux_state;

//////////////////////////////////////////////////////////////////////

[[nodiscard]] static const char* platform_get_error_name(uint8_t error_code)
{
    switch (error_code) 
    {
        case XCB_REQUEST: return "BAD REQUEST";
        case XCB_VALUE: return "BAD VALUE";
        case XCB_WINDOW: return "BAD WINDOW";
        case XCB_PIXMAP: return "BAD PIXMAP";
        case XCB_ATOM: return "BAD ATOM";
        case XCB_CURSOR: return "BAD CURSOR";
        case XCB_FONT: return "BAD FONT";
        case XCB_MATCH: return "BAD MATCH";
        case XCB_DRAWABLE: return "BAD DRAWABLE";
        case XCB_ACCESS : return "BAD ACCESS";
        case XCB_ALLOC : return "BAD ALLOC";
        case XCB_COLORMAP : return "BAD COLORMAP";
        case XCB_G_CONTEXT : return "BAD G CONTEXT";
        case XCB_ID_CHOICE : return "BAD ID CHOICE";
        case XCB_NAME : return "BAD NAME";
        case XCB_LENGTH : return "BAD LENGTH";
        case XCB_IMPLEMENTATION : return "BAD IMPLEMENTATION";
    }

    return "";
}

[[nodiscard]] static b8 platform_result_is_success(xcb_void_cookie_t cookie)
{
    xcb_generic_error_t* error = xcb_request_check(linux_state.connection, cookie);
    if (error != NULL)
    {
        fprintf(stderr, "[%s] %i", platform_get_error_name(error->error_code), error->full_sequence);
        return false;
    }

    return true;
}

//////////////////////////////////////////////////////////////////////

bool platform_startup()
{
    linux_state.display = XOpenDisplay(NULL);
    if (linux_state.display == NULL)
    {
        fprintf(stderr, "Failed to open display\n");
        return false;
    }

    linux_state.connection = XGetXCBConnection(linux_state.display);
    if (xcb_connection_has_error(linux_state.connection))  
    {
        fprintf(stderr, "Failed to connect to X server.\n");
        return false;
    }

    const xcb_setup_t* setup = xcb_get_setup(linux_state.connection);
    xcb_screen_iterator_t iter = xcb_setup_roots_iterator(setup);
    linux_state.screen = iter.data;

    linux_state.window = xcb_generate_id(linux_state.connection);
    u32 value_mask = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK;
    u32 value_list[] = { linux_state.screen->black_pixel, 
        XCB_EVENT_MASK_EXPOSURE | 
        XCB_EVENT_MASK_KEY_PRESS | XCB_EVENT_MASK_KEY_RELEASE | 
        XCB_EVENT_MASK_BUTTON_PRESS | XCB_EVENT_MASK_BUTTON_RELEASE | 
        XCB_EVENT_MASK_POINTER_MOTION };

    s32 window_x = 500;
    s32 window_y = 500;
    s32 window_pos[2] = { window_x, window_y };
    s32 window_width = 100;
    s32 window_height = 100;
    s32 border_width = 0;

    if (!platform_result_is_success(xcb_create_window_checked(
                    linux_state.connection,              // X server connection
                    CopyFromParent,                      // Window depth
                    linux_state.window,                  // Window ID
                    linux_state.screen->root,            // Parent window (root)
                    window_x, window_y,                  // Position (x, y)
                    window_width, window_height,         // Size (width, height)
                    border_width,                        // Border width
                    XCB_WINDOW_CLASS_INPUT_OUTPUT,       // Window class
                    linux_state.screen->root_visual,     // Visual
                    value_mask,                          // Value mask
                    value_list)))                        // Value list
    {
        fprintf(stderr, "Failed to create window.\n");
        return false;
    }
   
    xcb_configure_window(linux_state.connection, 
            linux_state.window, 
            XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y, 
            window_pos); 

    if (!platform_result_is_success(xcb_map_window_checked(
                    linux_state.connection, 
                    linux_state.window)))
    {
        fprintf(stderr, "Failed to map window.\n");
    }
    xcb_flush(linux_state.connection);

    if (!platform_get_window_info(&linux_state.window_info))
    {
        // TODO: logging
        return false;
    }

    return true;
}

void platform_shutdown()
{
    xcb_disconnect(linux_state.connection);
}

void platform_process_input()
{
    xcb_generic_event_t* event;
    while ((event = xcb_poll_for_event(linux_state.connection)))
    {
        switch (event->response_type & ~0x80) 
        {
            case XCB_KEY_PRESS:
            case XCB_KEY_RELEASE:
            {
                xcb_key_press_event_t* key_event = reinterpret_cast<xcb_key_press_event_t*>(event);
                b8 pressed = key_event->response_type == XCB_KEY_PRESS; 
                xcb_keycode_t code = key_event->detail;
                KeySym key_sym = XkbKeycodeToKeysym(linux_state.display, code, 0, 0);
                keycode_t keycode = translate_keycode(key_sym);

                linux_state.keyboard_event(keycode, pressed);
                
                if (keycode == KEY_ESCAPE)
                {
                    platform_shutdown();
                }
            } break; 

            case XCB_BUTTON_PRESS:
            case XCB_BUTTON_RELEASE:
            {
                xcb_button_press_event_t* mouse_event = reinterpret_cast<xcb_button_press_event_t*>(event);
                b8 pressed = mouse_event->response_type == XCB_BUTTON_PRESS;
                mouse_button_t mouse_button = {}; 
                switch (mouse_event->detail)
                {
                    case PLATFORM_MOUSE_BUTTON_LEFT:
                    {
                        mouse_button = MOUSE_BUTTON_LEFT;
                    } break;
                    case PLATFORM_MOUSE_BUTTON_MIDDLE:
                    {
                        mouse_button = MOUSE_BUTTON_MIDDLE;
                    } break;
                    case PLATFORM_MOUSE_BUTTON_RIGHT:
                    {
                        mouse_button = MOUSE_BUTTON_RIGHT;
                    } break;
                    case PLATFORM_MOUSE_BUTTON_1:
                    {
                        mouse_button = MOUSE_BUTTON_1;
                    } break;
                    case PLATFORM_MOUSE_BUTTON_2:
                    {
                        mouse_button = MOUSE_BUTTON_2;
                    } break;
                    case PLATFORM_MOUSE_BUTTON_3:
                    {
                        mouse_button = MOUSE_BUTTON_3;
                    } break;
                    case PLATFORM_MOUSE_BUTTON_4:
                    {
                        mouse_button = MOUSE_BUTTON_4;
                    } break;
                    case PLATFORM_MOUSE_BUTTON_5:
                    {
                        mouse_button = MOUSE_BUTTON_5;
                    } break;
                    case PLATFORM_MOUSE_BUTTON_6:
                    {
                        mouse_button = MOUSE_BUTTON_6;
                    } break;
                    case PLATFORM_MOUSE_BUTTON_7:
                    {
                        mouse_button = MOUSE_BUTTON_7;
                    } break;
                    case PLATFORM_MOUSE_BUTTON_8:
                    {
                        mouse_button = MOUSE_BUTTON_8;
                    } break;
                    case PLATFORM_MOUSE_BUTTON_9:
                    {
                        mouse_button = MOUSE_BUTTON_9;
                    } break;
                    case PLATFORM_MOUSE_WHEEL_LEFT:
                    {
                        mouse_button = MOUSE_WHEEL_LEFT;
                    } break;
                    case PLATFORM_MOUSE_WHEEL_RIGHT:
                    {
                        mouse_button = MOUSE_WHEEL_RIGHT;
                    } break;
                                        
                    linux_state.mouse_button_event(mouse_button, pressed);
                }

                u32 delta = 0;
                switch (mouse_event->detail)
                {
                    case PLATFORM_MOUSE_WHEEL_UP:
                    {
                        delta = pressed ? 1 : 0;
                 
                    } break;
                    case PLATFORM_MOUSE_WHEEL_DOWN:
                    {
                        delta = pressed ? -1 : 0;
                    } break;
               
                    linux_state.mouse_wheel_event(delta);
                }
            } break;

            case XCB_MOTION_NOTIFY:
            {
                xcb_motion_notify_event_t* motion_event = reinterpret_cast<xcb_motion_notify_event_t*>(event);

                linux_state.mouse_move_event(motion_event->event_x, motion_event->event_y);
            } break;

            case XCB_CONFIGURE_NOTIFY:
            {
                xcb_configure_notify_event_t* configure_event = reinterpret_cast<xcb_configure_notify_event_t*>(event);
            
                u16 width = configure_event->width;
                u16 height = configure_event->height;

                // TODO: reszie window
            } break;
        
            case XCB_CLIENT_MESSAGE:
            {
                xcb_client_message_event_t* client_event = reinterpret_cast<xcb_client_message_event_t*>(event);

                if (client_event->data.data32[0] == 0)
                {
                    platform_shutdown();
                }
            } break;
        }
        free(event);
    }
}

[[nodiscard]] b8 platform_load_library(const char* path, library_context_t* out_library_context)
{
    void* library_handle = dlopen(path, RTLD_NOW);
    if (!library_handle)
    {
        fprintf(stderr, "Failed to load library: %s\n", dlerror());
        return false;
    }

    out_library_context->handle = library_handle;
    return true;
}

[[nodiscard]] b8 platform_unload_library(library_context_t* library_context)
{
    if (dlclose(library_context->handle) != 0)
    {
        fprintf(stderr, "Failed to unload library: %s\n", dlerror());
        return false;
    }

    for (function_description_t* function_description : library_context->functions)
    {
        function_description->function = nullptr;
        function_description->is_dirty = true;
        function_description->library_handle = nullptr;
    }

    library_context->handle = nullptr;
    return true;
}

[[nodiscard]] b8 platform_get_function(library_context_t* library_context, function_description_t* out_function_description)
{
    out_function_description->function = dlsym(library_context->handle, out_function_description->name);
    if (!out_function_description->function)
    {
        fprintf(stderr, "Failed to load function: %s\n", dlerror());
        return false;
    }
    out_function_description->is_dirty = false;
    out_function_description->library_handle = library_context->handle;
    out_function_description->total_execution_time = 0.0f;
    out_function_description->average_execution_time = 0.0f;
    library_context->functions.emplace_back(out_function_description);    

    fprintf(stderr, "Loaded function successfully.\n");
    return true; 
}

// FIXME: window pos is necessary and also check if window is on top!
[[nodiscard]] b8 platform_is_mouse_inside_window()
{
    Window root_return;
    Window child_return;
    s32 root_x, root_y;
    s32 win_x, win_y;
    u32 mask_return;

    if (XQueryPointer(linux_state.display, linux_state.window, &root_return, &child_return, 
                &root_x, &root_y, &win_x, &win_y, &mask_return))
    {
        return win_x >= 0 && 
               win_y >= 0 && 
               win_x < linux_state.window_info.window_width 
               && win_y < linux_state.window_info.window_height;
    }

    return false;
}

[[nodiscard]] b8 platform_set_window_mode(platform_window_mode_t platform_window_mode)
{
    Atom wm_state_atom = XInternAtom(linux_state.display, "_NET_WM_STATE", False);
    Atom wm_state_fullscreen_atom = XInternAtom(linux_state.display, "_NET_WM_STATE_FULLSCREEN", False);
    Atom motif_wm_hints_atom = XInternAtom(linux_state.display, "_MOTIF_WM_HINTS", True);
    
    if (!platform_get_window_info(&linux_state.window_info))
    {
        fprintf(stderr, "Failed to save window info.\n");
        return false;
    }

    switch (platform_window_mode) 
    {
        case FULLSCREEN:
        {
            XEvent event = {};
            event.type = ClientMessage;
            event.xclient.window = linux_state.window;
            event.xclient.message_type = wm_state_atom;
            event.xclient.format = 32;
            event.xclient.data.l[0] = 1;
            event.xclient.data.l[1] = wm_state_fullscreen_atom;
            event.xclient.data.l[2] = 0;
            XSendEvent(linux_state.display, DefaultRootWindow(linux_state.display), False, 
                    SubstructureNotifyMask | SubstructureRedirectMask, &event);
        } break;
       
        case WINDOWED:
        default:
        {
            XEvent event = {};
            event.type = ClientMessage;
            event.xclient.window = linux_state.window;
            event.xclient.message_type = wm_state_atom;
            event.xclient.format = 32;
            event.xclient.data.l[0] = 0;
            event.xclient.data.l[1] = wm_state_fullscreen_atom;
            event.xclient.data.l[2] = 0;
            XSendEvent(linux_state.display, DefaultRootWindow(linux_state.display), False, 
                    SubstructureNotifyMask | SubstructureRedirectMask, &event);

            linux_window_mode_hints_t windowed_hints = { 2, 0, 1, 0, 0 };
            XChangeProperty(linux_state.display, linux_state.window, motif_wm_hints_atom, motif_wm_hints_atom, 
                    32, PropModeReplace, (unsigned char*)&windowed_hints, 5);

            XMoveResizeWindow(linux_state.display, 
                    linux_state.window, 
                    linux_state.window_info.x, linux_state.window_info.y, 
                    linux_state.window_info.window_width, linux_state.window_info.window_height);
        } break;
    }

    XFlush(linux_state.display);
    XSetInputFocus(linux_state.display, linux_state.window, RevertToParent, CurrentTime);

    return true;
}

[[nodiscard]] b8 platform_get_window_info(platform_window_info_t* platform_window_info)
{
    if (!platform_window_info)
    {
        fprintf(stderr, "Failed to provide window info! platform_window_info_t is nullptr.\n");
        return false;
    }

    XWindowAttributes attributes;
    if (!XGetWindowAttributes(linux_state.display, linux_state.window, &attributes))
    {
        fprintf(stderr, "Failed to retrieve window attributes.\n");
        return false;
    }
    platform_window_info->client_width = static_cast<s16>(attributes.width);
    platform_window_info->client_height = static_cast<s16>(attributes.height);

    s32 dest_x_result;
    s32 dest_y_result;
    Window child_result;
    if (!XTranslateCoordinates(linux_state.display, linux_state.window, attributes.root,
                0, 0, &dest_x_result, &dest_y_result, &child_result))
    {
        fprintf(stderr, "Failed to translate coordinates.\n");
        return false;
    }
    platform_window_info->x = dest_x_result;
    platform_window_info->y = dest_y_result;

    Atom net_frame_extents = XInternAtom(linux_state.display, "_NET_FRAME_EXTENTS", True);
    Atom actual_type;
    int actual_format;
    unsigned long nitems, bytes_after;
    long* extents = NULL;

    if (net_frame_extents &&
        XGetWindowProperty(linux_state.display, 
            attributes.root, net_frame_extents, 0, 4, False, XA_CARDINAL,
            &actual_type, &actual_format, 
            &nitems, &bytes_after, (unsigned char**)&extents) == Success && extents) 
    {
        s16 left = (s16)extents[0];
        s16 right = (s16)extents[1];
        s16 top = (s16)extents[2];
        s16 bottom = (s16)extents[3];

        platform_window_info->window_width = platform_window_info->client_width + left + right;
        platform_window_info->window_height = platform_window_info->client_height + top + bottom;

        free(extents);
    } 
    else 
    {
        platform_window_info->window_width = platform_window_info->client_width;
        platform_window_info->window_height = platform_window_info->client_height;
    }

#if false
    // 4. Sichtbarkeit prüfen
    info->is_visible = (attributes.map_state == IsViewable);

    // 5. Fokus prüfen (_NET_ACTIVE_WINDOW)
    Atom net_active_window = XInternAtom(display, "_NET_ACTIVE_WINDOW", True);
    if (net_active_window) {
        Atom actual_type;
        int actual_format;
        unsigned long num_items, bytes_after;
        unsigned char* prop = NULL;

        if (XGetWindowProperty(display, attributes.root, net_active_window, 0, 1, False, XA_WINDOW,
                               &actual_type, &actual_format, &num_items, &bytes_after, &prop) == Success && prop) {
            Window active_window = *(Window*)prop;
            info->is_focused = (active_window == window);
            XFree(prop);
        }
    }

    // 6. Monitorinformationen
    // Vereinfachte Annahme: Monitor 0 ist der gesamte Bildschirm
    // Für Multi-Monitor: Hole Monitorgrenzen (z. B. mit RandR oder Xinerama)
    info->monitor_index = 0;

    // 7. DPI-Skalierung berechnen
    int screen = DefaultScreen(display);
    double dpi = ((double)DisplayWidth(display, screen) / (double)DisplayWidthMM(display, screen)) * 25.4;
    info->dpi_scale = (float)(dpi / 96.0); // Skalierung relativ zu 96 DPI

    return true;
}
#endif

#if false
    s16 x;
    s16 y;
    s16 window_width;
    s16 window_height;
    s16 client_width;
    s16 client_height;
    b8 is_visible;
    b8 is_focused;
    s16 monitor_index;
    f32 dpi_scale;
    platform_window_mode_t platform_window_mode;
#endif

    return true;
}

void platform_register_keyboard_event(platform_keyboard_event_t callback)
{
    linux_state.keyboard_event = callback;
}

void platform_register_mouse_button_event(platform_mouse_button_event_t callback)
{
    linux_state.mouse_button_event = callback;
}

void platform_register_mouse_move_event(platform_mouse_move_event_t callback)
{
    linux_state.mouse_move_event = callback;
}

void platform_register_mouse_wheel_event(platform_mouse_wheel_event_t callback)
{
    linux_state.mouse_wheel_event = callback;
}

[[nodiscard]] static keycode_t translate_keycode(const unsigned int key_code)
{
    switch (key_code)
    {
        default:
        case XK_Escape:
            return KEY_ESCAPE;
        case XK_F1:
            return KEY_F1;
        case XK_F2:
            return KEY_F2;
        case XK_F3:
            return KEY_F3;
        case XK_F4:
            return KEY_F4;
        case XK_F5:
            return KEY_F5;
        case XK_F6:
            return KEY_F6;
        case XK_F7:
            return KEY_F7;
        case XK_F8:
            return KEY_F8;
        case XK_F9:
            return KEY_F9;
        case XK_F10:
            return KEY_F10;
        case XK_F11:
            return KEY_F11;
        case XK_F12:
            return KEY_F12;
        case XK_Insert:
            return KEY_INSERT;
        case XK_Delete:
            return KEY_DELETE;
        case XK_Home:
            return KEY_HOME;
        case XK_End:
            return KEY_END;
        case XK_Page_Up:
            return KEY_PAGE_UP;
        case XK_Page_Down:
            return KEY_PAGE_DOWN;
        case XK_dead_circumflex:
            return KEY_CIRCUMFLEX;
        case XK_0:
            return KEY_0;
        case XK_1:
            return KEY_1;
        case XK_2:
            return KEY_2;
        case XK_3:
            return KEY_3;
        case XK_4:
            return KEY_4;
        case XK_5:
            return KEY_5;
        case XK_6:
            return KEY_6;
        case XK_7:
            return KEY_7;
        case XK_8:
            return KEY_8;
        case XK_9:
            return KEY_9;
        case XK_ssharp:
            return KEY_SSHARP;
        case XK_acute:
            return KEY_ACUTE;
        case XK_Tab:
            return KEY_TAB;
        case XK_Caps_Lock:
            return KEY_CAPS_LOCK;
        case XK_Shift_L:
            return KEY_SHIFT_L;
        case XK_less:
            return KEY_LESS;
        case XK_Control_L:
            return KEY_CONTROL_L;
        case XK_Alt_L:
            return KEY_ALT_L;
        case XK_space:
            return KEY_SPACE;
        case XK_ISO_Level3_Shift:
            return KEY_ISO_LEVEL3_SHIFT;
        case XK_Menu:
            return KEY_MENU;
        case XK_Control_R:
            return KEY_CONTROL_R;
        case XK_Shift_R:
            return KEY_SHIFT_R;
        case XK_Return:
            return KEY_RETURN;
        case XK_BackSpace:
            return KEY_BACKSPACE;
        case XK_comma:
            return KEY_COMMA;
        case XK_period:
            return KEY_PERIOD;
        case XK_minus:
            return KEY_MINUS;
        case XK_Left:
            return KEY_LEFT;
        case XK_Up:
            return KEY_UP;
        case XK_Down:
            return KEY_DOWN;
        case XK_Right:
            return KEY_RIGHT;
        case XK_Num_Lock:
            return KEY_NUM_LOCK;
        case XK_KP_Divide:
            return KEY_KP_DIVIDE;
        case XK_KP_Multiply:
            return KEY_KP_MULTIPLY;
        case XK_KP_Subtract:
            return KEY_KP_SUBTRACT;
        case XK_KP_Add:
            return KEY_KP_ADD;
        case XK_KP_1:
        case XK_KP_End:
            return KEY_KP_1;
        case XK_KP_2:
        case XK_KP_Down:
            return KEY_KP_2;
        case XK_KP_3:
        case XK_KP_Page_Down:
            return KEY_KP_3;
        case XK_KP_4:
        case XK_KP_Left:
            return KEY_KP_4;
        case XK_KP_5:
        case XK_KP_Begin:
            return KEY_KP_5;
        case XK_KP_6:
        case XK_KP_Right:
            return KEY_KP_6;
        case XK_KP_7:
        case XK_KP_Home:
            return KEY_KP_7;
        case XK_KP_8:
        case XK_KP_Up:
            return KEY_KP_8;
        case XK_KP_9:
        case XK_KP_Page_Up:
            return KEY_KP_9;
        case XK_KP_Delete:
            return KEY_KP_DELETE;
        case XK_KP_Enter:
            return KEY_KP_ENTER;
        case XK_a:
        case XK_A:
            return KEY_A;
        case XK_b:
        case XK_B:
            return KEY_B;
        case XK_c:
        case XK_C:
            return KEY_C;
        case XK_d:
        case XK_D:
            return KEY_D;
        case XK_e:
        case XK_E:
            return KEY_E;
        case XK_f:
        case XK_F:
            return KEY_F;
        case XK_g:
        case XK_G:
            return KEY_G;
        case XK_h:
        case XK_H:
            return KEY_H;
        case XK_i:
        case XK_I:
            return KEY_I;
        case XK_j:
        case XK_J:
            return KEY_J;
        case XK_k:
        case XK_K:
            return KEY_K;
        case XK_l:
        case XK_L:
            return KEY_L;
        case XK_m:
        case XK_M:
            return KEY_M;
        case XK_n:
        case XK_N:
            return KEY_N;
        case XK_o:
        case XK_O:
            return KEY_O;
        case XK_p:
        case XK_P:
            return KEY_P;
        case XK_q:
        case XK_Q:
            return KEY_Q;
        case XK_r:
        case XK_R:
            return KEY_R;
        case XK_s:
        case XK_S:
            return KEY_S;
        case XK_t:
        case XK_T:
            return KEY_T;
        case XK_u:
        case XK_U:
            return KEY_U;
        case XK_v:
        case XK_V:
            return KEY_V;
        case XK_w:
        case XK_W:
            return KEY_W;
        case XK_x:
        case XK_X:
            return KEY_X;
        case XK_y:
        case XK_Y:
            return KEY_Y;
        case XK_z:
        case XK_Z:
            return KEY_Z;
    }
}

#include "warpunk.core/defines.h"
#if defined(WARPUNK_LINUX) 

#include "warpunk.core/platform/platform.h"
#include "warpunk.core/platform/platform_linux.h"

#include <cassert>
#include <cstdlib>
#include <cstdio>
#include <dlfcn.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>
#include <xcb/xcb.h>
#include <xcb/xproto.h>
#include <X11/X.h>
#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/Xlib-xcb.h>
#include <X11/XKBlib.h>
#include <X11/keysym.h>
#include <X11/keysymdef.h>

#include "warpunk.core/input_system/input_types.h"
#include "warpunk.core/container/stcqueue.hpp"
#include "warpunk.core/container/dynqueue.hpp"
#include "warpunk.core/container/dynarray.hpp"

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

#define PLATFORM_THREADPOOL_THREAD_COUNT 32

static keycode_t translate_keycode(const unsigned int key_code);
static void* platform_thread_main_routine(void* args);

//////////////////////////////////////////////////////////////////////

typedef struct _thread_handle_t
{
    thread_ticket_t ticket;
    s32 thread_idx;
} thread_handle_t;

typedef struct _thread_context_t
{
    dynarray_t<pthread_t> threads;
    dynarray_t<platform_threading_job_t> jobs;
    dynarray_t<thread_handle_t> handles;
    s64 active_thread_count;
    pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER; 
} thread_context_t;

typedef struct _linux_state_t
{
    Display* display;
    linux_handle_s handle;

    pthread_mutex_t mutex;
    b8 thread_ticket_free_list[64] = { true };
    thread_context_t thread_contexts[64];


    platform_keyboard_event_t keyboard_event;
    platform_mouse_button_event_t mouse_button_event;
    platform_mouse_move_event_t mouse_move_event;
    platform_mouse_wheel_event_t mouse_wheel_event;
} linux_state_t;

//////////////////////////////////////////////////////////////////////

// NOTE: Global

static linux_state_t linux_state;

//////////////////////////////////////////////////////////////////////

static const char* platform_get_error_name(uint8_t error_code)
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

b8 platform_result_is_success(xcb_void_cookie_t cookie)
{
    xcb_generic_error_t* error = xcb_request_check(linux_state.handle.connection, cookie);
    if (error != NULL)
    {
        fprintf(stderr, "[%s] %i", platform_get_error_name(error->error_code), error->full_sequence);
        return false;
    }

    return true;
}

b8 platform_get_linux_handle(linux_handle_s *out_linux_handle)
{
    if (linux_state.handle.connection == nullptr || linux_state.handle.window == 0 || linux_state.handle.screen == nullptr)
    {
        out_linux_handle = nullptr;
        return false;
    }

    *out_linux_handle = linux_state.handle;
    return true;
}

xcb_atom_t platform_get_atom(const char* name)
{
    xcb_intern_atom_cookie_t cookie = xcb_intern_atom(linux_state.handle.connection, 0, strlen(name), name);
    xcb_intern_atom_reply_t* reply = xcb_intern_atom_reply(linux_state.handle.connection, cookie, NULL);
    if (!reply) 
    {
        fprintf(stderr, "Failed to get atom for %s\n", name);
        return XCB_ATOM_NONE;
    }

    xcb_atom_t atom = reply->atom;
    free(reply);
    
    return atom;
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

    linux_state.handle.connection = XGetXCBConnection(linux_state.display);
    if (xcb_connection_has_error(linux_state.handle.connection))  
    {
        fprintf(stderr, "Failed to connect to X server.\n");
        return false;
    }

    const xcb_setup_t* setup = xcb_get_setup(linux_state.handle.connection);
    xcb_screen_iterator_t iter = xcb_setup_roots_iterator(setup);
    linux_state.handle.screen = iter.data;

    fprintf(stderr, "%i\n", linux_state.handle.screen->root_depth);

    linux_state.handle.window = xcb_generate_id(linux_state.handle.connection);
    u32 value_mask = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK;
    u32 value_list[] = { linux_state.handle.screen->black_pixel, 
        XCB_EVENT_MASK_EXPOSURE | 
        XCB_EVENT_MASK_KEY_PRESS | XCB_EVENT_MASK_KEY_RELEASE | 
        XCB_EVENT_MASK_BUTTON_PRESS | XCB_EVENT_MASK_BUTTON_RELEASE | 
        XCB_EVENT_MASK_POINTER_MOTION | XCB_EVENT_MASK_STRUCTURE_NOTIFY };

    s32 window_width = 960;
    s32 window_height = 540;
    s32 border_width = 0;

    if (!platform_result_is_success(xcb_create_window_checked(
                    linux_state.handle.connection,       // X server connection
                    XCB_COPY_FROM_PARENT,                // Window depth
                    linux_state.handle.window,           // Window ID
                    linux_state.handle.screen->root,            // Parent window (root)
                    0, 0,                                // Position (x, y)
                    window_width, window_height,         // Size (width, height)
                    border_width,                        // Border width
                    XCB_WINDOW_CLASS_INPUT_OUTPUT,       // Window class
                    linux_state.handle.screen->root_visual,     // Visual
                    value_mask,                          // Value mask
                    value_list)))                        // Value list
    {
        fprintf(stderr, "Failed to create window.\n");
        return false;
    }
   
    if (!platform_result_is_success(xcb_map_window_checked(
                    linux_state.handle.connection, 
                    linux_state.handle.window)))
    {
        fprintf(stderr, "Failed to map window.\n");
    }

    xcb_flush(linux_state.handle.connection);

    /** threading */
    pthread_mutex_init(&linux_state.mutex, nullptr);

    return true;
}

void platform_shutdown()
{
    xcb_disconnect(linux_state.handle.connection);
}

void platform_process_input()
{
    xcb_generic_event_t* event;
    while ((event = xcb_poll_for_event(linux_state.handle.connection)))
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
                        linux_state.mouse_button_event(mouse_button, pressed);
                    } break;
                    case PLATFORM_MOUSE_BUTTON_MIDDLE:
                    {
                        mouse_button = MOUSE_BUTTON_MIDDLE;
                        linux_state.mouse_button_event(mouse_button, pressed);
                    } break;
                    case PLATFORM_MOUSE_BUTTON_RIGHT:
                    {
                        mouse_button = MOUSE_BUTTON_RIGHT;
                        linux_state.mouse_button_event(mouse_button, pressed);
                    } break;
                    case PLATFORM_MOUSE_BUTTON_1:
                    {
                        mouse_button = MOUSE_BUTTON_1;
                        linux_state.mouse_button_event(mouse_button, pressed);
                    } break;
                    case PLATFORM_MOUSE_BUTTON_2:
                    {
                        mouse_button = MOUSE_BUTTON_2;
                        linux_state.mouse_button_event(mouse_button, pressed);
                    } break;
                    case PLATFORM_MOUSE_BUTTON_3:
                    {
                        mouse_button = MOUSE_BUTTON_3;
                        linux_state.mouse_button_event(mouse_button, pressed);
                    } break;
                    case PLATFORM_MOUSE_BUTTON_4:
                    {
                        mouse_button = MOUSE_BUTTON_4;
                        linux_state.mouse_button_event(mouse_button, pressed);
                    } break;
                    case PLATFORM_MOUSE_BUTTON_5:
                    {
                        mouse_button = MOUSE_BUTTON_5;
                        linux_state.mouse_button_event(mouse_button, pressed);
                    } break;
                    case PLATFORM_MOUSE_BUTTON_6:
                    {
                        mouse_button = MOUSE_BUTTON_6;
                        linux_state.mouse_button_event(mouse_button, pressed);
                    } break;
                    case PLATFORM_MOUSE_BUTTON_7:
                    {
                        mouse_button = MOUSE_BUTTON_7;
                        linux_state.mouse_button_event(mouse_button, pressed);
                    } break;
                    case PLATFORM_MOUSE_BUTTON_8:
                    {
                        mouse_button = MOUSE_BUTTON_8;
                        linux_state.mouse_button_event(mouse_button, pressed);
                    } break;
                    case PLATFORM_MOUSE_BUTTON_9:
                    {
                        mouse_button = MOUSE_BUTTON_9;
                        linux_state.mouse_button_event(mouse_button, pressed);
                    } break;
                    case PLATFORM_MOUSE_WHEEL_LEFT:
                    {
                        mouse_button = MOUSE_WHEEL_LEFT;
                        linux_state.mouse_button_event(mouse_button, pressed);
                    } break;
                    case PLATFORM_MOUSE_WHEEL_RIGHT:
                    {
                        mouse_button = MOUSE_WHEEL_RIGHT;
                        linux_state.mouse_button_event(mouse_button, pressed);
                    } break;                        
                }

                u32 delta = 0;
                switch (mouse_event->detail)
                {
                    case PLATFORM_MOUSE_WHEEL_UP:
                    {
                        delta = pressed ? 1 : 0; 
                        linux_state.mouse_wheel_event(delta);
                    } break;
                    case PLATFORM_MOUSE_WHEEL_DOWN:
                    {
                        delta = pressed ? -1 : 0;
                        linux_state.mouse_wheel_event(delta);
                    } break;
               
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

b8 platform_get_window_handle(s32* out_size, void* out_platform_handle)
{
    *out_size = sizeof(linux_handle_s);
    if (!out_platform_handle)
    {
        return false; 
    }

    // TODO: Platform allocation methods 
    memcpy(out_platform_handle, &linux_state.handle, sizeof(linux_handle_s));
    return true;
}

b8 platform_load_library(const char* path, library_context_t* out_library_context)
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

b8 platform_unload_library(library_context_t* library_context)
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

b8 platform_get_function(library_context_t* library_context, function_description_t* out_function_description)
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

b8 platform_is_mouse_inside_window()
{
    xcb_connection_t* connection = linux_state.handle.connection;
    xcb_window_t window = linux_state.handle.window;

    xcb_query_pointer_cookie_t pointer_cookie = xcb_query_pointer(connection, window);
    xcb_query_pointer_reply_t* pointer_reply = xcb_query_pointer_reply(connection, pointer_cookie, NULL);

    if (!pointer_reply) 
    {
        fprintf(stderr, "Failed to query pointer.\n");
        return false;
    }

    // Get window geometry
    xcb_get_geometry_cookie_t geom_cookie = xcb_get_geometry(connection, window);
    xcb_get_geometry_reply_t* geom_reply = xcb_get_geometry_reply(connection, geom_cookie, NULL);

    if (!geom_reply)
    {
        fprintf(stderr, "Failed to get window geometry.\n");
        free(pointer_reply);
        return false;
    }

    bool inside = (pointer_reply->same_screen &&
                   pointer_reply->win_x >= 0 && pointer_reply->win_x < geom_reply->width &&
                   pointer_reply->win_y >= 0 && pointer_reply->win_y < geom_reply->height);

    free(pointer_reply);
    free(geom_reply);
    return inside;
}

b8 platform_set_window_mode(platform_window_mode_t platform_window_mode)
{
    xcb_connection_t* connection = linux_state.handle.connection;
    xcb_window_t window = linux_state.handle.window;
    xcb_screen_t* screen = linux_state.handle.screen;

    xcb_atom_t wm_state_atom = platform_get_atom("_NET_WM_STATE");
    xcb_atom_t wm_state_fullscreen_atom = platform_get_atom("_NET_WM_STATE_FULLSCREEN");
    xcb_atom_t motif_wm_hints_atom = platform_get_atom("_MOTIF_WM_HINTS");

    switch (platform_window_mode) 
    {
        case FULLSCREEN:
        {
            xcb_client_message_event_t event = {};
            event.response_type = XCB_CLIENT_MESSAGE;
            event.window = window;
            event.type = wm_state_atom;
            event.format = 32;
            event.data.data32[0] = 1; // _NET_WM_STATE_ADD
            event.data.data32[1] = wm_state_fullscreen_atom;
            event.data.data32[2] = 0;

            xcb_send_event(connection, 
                    false, 
                    screen->root,
                    XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY | XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT,
                    (const char*)&event);
        } break;
       
        case WINDOWED:
        default:
        {
            xcb_client_message_event_t event = {};
            event.response_type = XCB_CLIENT_MESSAGE;
            event.window = window;
            event.type = wm_state_atom;
            event.format = 32;
            event.data.data32[0] = 0; 
            event.data.data32[1] = wm_state_fullscreen_atom;
            event.data.data32[2] = 0;

            xcb_send_event(connection, 
                    false, 
                    screen->root,
                    XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY | XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT,
                    (const char*)&event);

        } break;
    }

    xcb_flush(connection);
    xcb_set_input_focus(connection, XCB_INPUT_FOCUS_PARENT, window, XCB_CURRENT_TIME);

    return true;
}

b8 platform_get_window_info(platform_window_info_t* platform_window_info)
{
    if (!platform_window_info)
    {
        fprintf(stderr, "Failed to provide window info! platform_window_info_t is nullptr.\n");
        return false;
    }

    xcb_connection_t* connection = linux_state.handle.connection;
    xcb_window_t window = linux_state.handle.window;

     // I. Get Geometry (width, height)
    xcb_get_geometry_cookie_t geom_cookie = xcb_get_geometry(connection, window);
    xcb_get_geometry_reply_t* geom_reply = xcb_get_geometry_reply(connection, geom_cookie, NULL);

    if (!geom_reply) 
    {
        fprintf(stderr, "Failed to get window geometry.\n");
        return false;
    }

    platform_window_info->width = geom_reply->width;
    platform_window_info->height = geom_reply->height;
    free(geom_reply);

    // II. Check visibility (is_visible)
    xcb_get_window_attributes_cookie_t attr_cookie = xcb_get_window_attributes(connection, window);
    xcb_get_window_attributes_reply_t* attr_reply = xcb_get_window_attributes_reply(connection, attr_cookie, NULL);

    if (!attr_reply) 
    {
        fprintf(stderr, "Failed to get window attributes.\n");
        return false;
    }

    platform_window_info->is_visible = (attr_reply->map_state == XCB_MAP_STATE_VIEWABLE);
    free(attr_reply);

    // III. DPI Scale (dpi_scale)
    // Here we assume a default DPI of 96 as base, scaling depending on monitor settings.
    // More complex setups may require XRandR or Wayland for accurate DPI.
    platform_window_info->dpi_scale = 1.0f; // Default DPI scale (could be updated with further queries)

    // IV. Monitor Index (monitor_index)
    // Simplification: Assuming a single monitor (0 index).
    platform_window_info->monitor_index = 0;

    // V. Title (title)
    // Get window name (_NET_WM_NAME if supported, fallback to WM_NAME)
    xcb_intern_atom_cookie_t net_wm_name_cookie = xcb_intern_atom(connection, 1, strlen("_NET_WM_NAME"), "_NET_WM_NAME");
    xcb_intern_atom_reply_t* net_wm_name_reply = xcb_intern_atom_reply(connection, net_wm_name_cookie, NULL);

    if (net_wm_name_reply) 
    {
        xcb_get_property_cookie_t name_cookie = xcb_get_property(
            connection, 0, window, net_wm_name_reply->atom, XCB_GET_PROPERTY_TYPE_ANY, 0, 128);

        xcb_get_property_reply_t* name_reply = xcb_get_property_reply(connection, name_cookie, NULL);

        if (name_reply) 
        {
            platform_window_info->title = strndup((char*)xcb_get_property_value(name_reply), xcb_get_property_value_length(name_reply));
            free(name_reply);
        } 
        else 
        {
            platform_window_info->title = strdup("Unknown");
        }
        free(net_wm_name_reply);
    } 
    else 
    {
        platform_window_info->title = strdup("Unknown");
    }

    // VI. Platform Window Mode (platform_window_mode)
    // Query _NET_WM_STATE to check if it's in fullscreen mode
    xcb_intern_atom_cookie_t wm_state_cookie = xcb_intern_atom(connection, 1, strlen("_NET_WM_STATE"), "_NET_WM_STATE");
    xcb_intern_atom_reply_t* wm_state_reply = xcb_intern_atom_reply(connection, wm_state_cookie, NULL);

    if (wm_state_reply) 
    {
        xcb_get_property_cookie_t state_cookie = xcb_get_property(
            connection, 0, window, wm_state_reply->atom, XCB_GET_PROPERTY_TYPE_ANY, 0, 32);

        xcb_get_property_reply_t* state_reply = xcb_get_property_reply(connection, state_cookie, NULL);

        if (state_reply) 
        {
            xcb_atom_t* atoms = (xcb_atom_t*)xcb_get_property_value(state_reply);
            int len = xcb_get_property_value_length(state_reply) / sizeof(xcb_atom_t);

            xcb_atom_t fullscreen_atom = platform_get_atom("_NET_WM_STATE_FULLSCREEN");
            platform_window_info->platform_window_mode = WINDOWED;

            for (int i = 0; i < len; i++) 
            {
                if (atoms[i] == fullscreen_atom) 
                {
                    platform_window_info->platform_window_mode = FULLSCREEN;
                    break;
                }
            }

            free(state_reply);
        }
        
        free(wm_state_reply);
    }

    return true;
}

/************/
/** memory **/
/************/

void* platform_memory_alloc(s64 size)
{
    void* memory = aligned_alloc(16, size);
    return memory;
}

void platform_memory_free(void* src)
{
    free(src);
}

void platform_memory_copy(void* dst, void* src, s64 size)
{
    memcpy(dst, src, size);
}

void platform_memory_set(void* dst, s64 size, s32 value)
{
    memset(dst, value, size);
}

void platform_memory_zero(void* dst, s64 size)
{
    memset(dst, 0, size);
}

/***************/
/** threading **/
/***************/

static void* platform_thread_main_routine(void* args)
{
    thread_handle_t* handle = (thread_handle_t *)args;
    thread_context_t* thread_context = &linux_state.thread_contexts[handle->ticket];
    platform_threading_job_t* job = (platform_threading_job_t *)&thread_context->jobs.data[handle->thread_idx]; 

    if (job != nullptr && job->function != nullptr)
    {
        job->function(job->arg);
        if (job->arg)
        {
            platform_memory_free(job->arg);
            job->arg = nullptr;
        }
    }

    pthread_mutex_lock(&thread_context->mutex);
    thread_context->active_thread_count--;
    if (thread_context->active_thread_count == 0)
    {
        dynarray_destroy(&thread_context->jobs);
        linux_state.thread_ticket_free_list[handle->ticket] = true;
        dynarray_destroy(&thread_context->handles);
    }
    pthread_mutex_unlock(&thread_context->mutex);
    return nullptr;
}

void platform_threadpool_add(platform_threading_job_t* jobs, u32 chunk_count, thread_ticket_t* out_ticket)
{
    pthread_mutex_lock(&linux_state.mutex);
    b8 found = false;
    thread_ticket_t ticket_idx;
    for (ticket_idx = 0; ticket_idx < 64; ++ticket_idx)
    {
        if (linux_state.thread_ticket_free_list[ticket_idx] == true)
        {
            linux_state.thread_ticket_free_list[ticket_idx] = false;
            found = true;
            break;
        }
    }
    if (found == false)
    {
        return;
    }

    *out_ticket = ticket_idx;

    thread_context_t* thread_context = &linux_state.thread_contexts[ticket_idx];
    if (thread_context->threads.data == nullptr)
    {
        thread_context->threads = dynarray_create<pthread_t>(chunk_count);
    }
    dynarray_clear(&thread_context->threads);
    thread_context->jobs = dynarray_create<platform_threading_job_t>(chunk_count);
    thread_context->handles = dynarray_create<thread_handle_t>(chunk_count);
    thread_context->active_thread_count = chunk_count;
    
    for (s32 job_idx = 0; job_idx < chunk_count; ++job_idx)
    {
        platform_threading_job_t job = {};
        job.function = jobs->function;
        job.arg_size = jobs->arg_size;
        job.arg = platform_memory_alloc(jobs->arg_size);
        platform_memory_copy(job.arg, ((u8 *)jobs->arg) + jobs->arg_size * job_idx, jobs->arg_size);
       
        thread_context->threads.data[job_idx] = {};
        thread_context->jobs.data[job_idx] = job;
        thread_context->handles.data[job_idx] = { ticket_idx, job_idx };
       
        pthread_create(&thread_context->threads.data[job_idx], 
                nullptr, 
                platform_thread_main_routine,
                &thread_context->handles.data[job_idx]);
    }
    pthread_mutex_unlock(&linux_state.mutex);
}
 
void platform_threadpool_sync(thread_ticket_t ticket, f64 cancellation_time)
{
    thread_context_t* thread_context = &linux_state.thread_contexts[ticket];
    for (s32 thread_idx = 0; thread_idx < thread_context->threads.size; ++thread_idx)
    {
        pthread_join(thread_context->threads.data[thread_idx], nullptr);
    }
}

/************/
/** events **/
/************/

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

static keycode_t translate_keycode(const unsigned int key_code)
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

#endif // WARPUNK_LINUX

#include "warpunk.core/renderer/platform/software_platform.h"

#include <stdlib.h>

#include "warpunk.core/platform/platform.h"
#include "warpunk.core/platform/platform_linux.h"

linux_handle_info_t* handle;
xcb_pixmap_t pixmap;
xcb_gcontext_t gcontext;

[[nodiscard]] b8 software_platform_startup()
{
    if (!handle)
    {
       // TODO: Platform allocation!
        handle = (linux_handle_info_t *)malloc(sizeof(linux_handle_info_t));
    }
    
    s32 size;
    if (!platform_get_window_handle(&size, handle))
    {
        return false;
    }

    uint64_t max_req_len = xcb_get_maximum_request_length(handle->connection);

    xcb_screen_t* screen = xcb_setup_roots_iterator(xcb_get_setup(handle->connection)).data;
    gcontext = xcb_generate_id(handle->connection);
    if (!platform_result_is_success(xcb_create_gc(
                    handle->connection, 
                    gcontext, 
                    handle->window, 
                    0, 
                    0)))
    {
        return false;
    }
    
    return true;
}

[[nodiscard]] b8 software_platform_submit_framebuffer(s32 width, s32 height, s32 size, u8* framebuffer)
{
    if (!handle)
    {
        return false;
    }

    pixmap = xcb_generate_id(handle->connection);
    xcb_create_pixmap(handle->connection, 24, pixmap, handle->window, width, height);

    if (!platform_result_is_success(xcb_put_image(
                    handle->connection, 
                    XCB_IMAGE_FORMAT_Z_PIXMAP, 
                    pixmap,
                    gcontext, 
                    width, height, 
                    0, 0, 
                    0, 
                    24, 
                    size, 
                    framebuffer)))
    {
        return false;
    }

    if (!platform_result_is_success(xcb_copy_area(
                    handle->connection,
                    pixmap,
                    handle->window,
                    gcontext,
                    0, 0, 0, 0,
                    width,
                    height
                    )))
    {
        return false;
    }

    xcb_flush(handle->connection);

    xcb_free_pixmap(handle->connection, pixmap);
    return true;
}

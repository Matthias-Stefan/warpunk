#pragma once

#include "warpunk.core/src/defines.h"
#include "warpunk.core/src/renderer/renderer_backend.h"

namespace software_renderer 
{
    //* */
    b8 renderer_startup(renderer_config renderer_config);

    /** */
    b8 renderer_shutdown();

    /** */
    void renderer_begin_frame();

    /** */
    void renderer_end_frame();

    /** */
    buffer_handle renderer_create_buffer(s32 size, void* data);

    /** */
    void renderer_destroy_buffer(buffer_handle buffer_handle);

    /** */
    texture_handle renderer_create_texture(s32 width, s32 height, void* data);

    /** */
    void renderer_destroy_texture(texture_handle texture_handle);

    /** */
    void renderer_draw(void* vertex_array, void* material);
}


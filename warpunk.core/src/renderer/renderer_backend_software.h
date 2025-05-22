#pragma once

#include "warpunk.core/src/defines.h"
#include "warpunk.core/src/renderer/renderer_backend.h"

namespace software_renderer 
{
    //* */
    b8 renderer_startup(renderer_config_s renderer_config);

    /** */
    b8 renderer_shutdown();

    /** */
    void renderer_begin_frame();

    /** */
    void renderer_end_frame();

    /** */
    buffer_handle_s renderer_create_buffer(s32 size, void* data);

    /** */
    void renderer_destroy_buffer(buffer_handle_s buffer_handle);

    /** */
    texture_handle_s renderer_create_texture(s32 width, s32 height, void* data);

    /** */
    void renderer_destroy_texture(texture_handle_s texture_handle);

    /** */
    void renderer_draw(void* vertex_array, void* material);
}


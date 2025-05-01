#pragma once

#include "warpunk.core/defines.h"
#include "warpunk.core/renderer/renderer_types.h"

typedef enum _renderer_type_t
{
    RENDERER_TYPE_SOFTWARE,
    RENDERER_TYPE_VULKAN
} renderer_type_t;

typedef struct _renderer_config_s
{
    renderer_type_t type;
    s32 width;
    f64 aspect_ratio;
    renderer_config_flag flags;
} renderer_config_s;

typedef u32 buffer_handle_s;
typedef u32 texture_handle_s;

//* */
warpunk_api b8 renderer_startup(renderer_config_s renderer_config);

/** */
warpunk_api b8 renderer_shutdown();

/** */
warpunk_api void renderer_begin_frame();

/** */
warpunk_api void renderer_end_frame();

/** */
warpunk_api buffer_handle_s renderer_create_buffer(s32 size, void* data);

/** */
warpunk_api void renderer_destroy_buffer(buffer_handle_s buffer_handle);

/** */
warpunk_api texture_handle_s renderer_create_texture(s32 width, s32 height, void* data);

/** */
warpunk_api void renderer_destroy_texture(texture_handle_s texture_handle);

/** */
warpunk_api void renderer_draw(void* vertex_array, void* material);




#pragma once

#include "warpunk.core/src/defines.h"
#include "warpunk.core/src/renderer/renderer_types.h"

typedef enum renderer_type
{
    RENDERER_TYPE_SOFTWARE,
    RENDERER_TYPE_VULKAN
} renderer_type;

typedef struct renderer_config
{
    renderer_type type;
    s32 width;
    f64 aspect_ratio;
    renderer_config_flag flags;
} renderer_config;

typedef u32 buffer_handle;
typedef u32 texture_handle;

//* */
warpunk_api b8 renderer_startup(renderer_config renderer_config);

/** */
warpunk_api b8 renderer_shutdown();

/** */
warpunk_api void renderer_begin_frame();

/** */
warpunk_api void renderer_end_frame();

/** */
warpunk_api buffer_handle renderer_create_buffer(s32 size, void* data);

/** */
warpunk_api void renderer_destroy_buffer(buffer_handle buffer_handle);

/** */
warpunk_api texture_handle renderer_create_texture(s32 width, s32 height, void* data);

/** */
warpunk_api void renderer_destroy_texture(texture_handle texture_handle);

/** */
warpunk_api void renderer_draw(void* vertex_array, void* material);




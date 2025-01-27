#pragma once

#include "warpunk.core/defines.h"

typedef enum _renderer_type_t
{
    RENDERER_TYPE_SOFTWARE
} renderer_type_t;

typedef struct _renderer_config_t
{
    renderer_type_t type;
    s32 width;
    s32 height;
} renderer_config_t;

typedef u32 buffer_handle_t;
typedef u32 texture_handle_t;

//* */
b8 renderer_startup(renderer_config_t renderer_config);

/** */
b8 renderer_shutdown();

/** */
warpunk_api void renderer_begin_frame();

/** */
warpunk_api void renderer_end_frame();

/** */
warpunk_api [[nodiscard]] buffer_handle_t renderer_create_buffer(s32 size, void* data);

/** */
warpunk_api void renderer_destroy_buffer(buffer_handle_t buffer_handle);

/** */
warpunk_api [[nodiscard]] texture_handle_t renderer_create_texture(s32 width, s32 height, void* data);

/** */
warpunk_api void renderer_destroy_texture(texture_handle_t texture_handle);

/** */
warpunk_api void renderer_draw(void* vertex_array, void* material);

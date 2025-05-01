#pragma once

#include "warpunk.core/defines.h"

typedef struct _camera_config_s
{
    f64 aspect_ratio;
    f64 focal_length;
    s32 image_width;
    f64 viewport_height;
    s32 samples_per_pixel;
    s32 max_depth;
} camera_config_s;

/** */
camera_handle_t camera_create(camera_config_s camera_config);

/** */
void camera_ray_cast(camera_handle_t camera_handle, void* objects, u8* out_buffer);


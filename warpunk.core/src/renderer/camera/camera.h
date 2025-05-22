#pragma once

#include "warpunk.core/src/defines.h"

typedef struct camera_config
{
    f64 aspect_ratio;
    f64 focal_length;
    s32 image_width;
    f64 viewport_height;
    s32 samples_per_pixel;
    s32 max_depth;
} camera_config;

/** */
camera_handle camera_create(camera_config camera_config);

/** */
void camera_ray_cast(camera_handle camera_handle, void* objects, u8* out_buffer);


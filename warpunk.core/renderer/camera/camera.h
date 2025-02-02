#pragma once

#include "warpunk.core/defines.h"

 typedef struct _camera_config_t
{
    f64 focal_length;
    s32 image_width;
    s32 image_height;
    f64 viewport_height;
} camera_config_t;


/** */
struct camera_t* camera_create(camera_config_t camera_config);

/** */
void camera_ray_cast(const struct camera_t* camera, void* objects);

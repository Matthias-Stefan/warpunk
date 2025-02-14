#include "warpunk.core/renderer/renderer_backend.h"

#include "warpunk.core/defines.h"
#include "warpunk.core/renderer/platform/software_platform.h"
#include "warpunk.core/renderer/camera/camera.h"

#include "warpunk.core/math/hittable.hpp"

#define IMAGE_WIDTH 1000
#define IMAGE_HEIGHT 600
#define BYTES_PER_PIXEL 4

static u8 framebuffer[IMAGE_WIDTH * IMAGE_HEIGHT * BYTES_PER_PIXEL];
static camera_handle_t camera_handle;

static sphere_t<f64> spheres[2];

[[nodiscard]] b8 renderer_startup(renderer_config_t renderer_config)
{
    if (!software_platform_startup())
    {
        return false;
    }

    /** camera */
    camera_config_t camera_config = {
        .aspect_ratio = 16.0 / 9.0,
        .focal_length = 1.0,
        .image_width = IMAGE_WIDTH,
        .viewport_height = 2.0,
        .samples_per_pixel = 64
    };
    camera_handle = camera_create(camera_config);

    /** spheres */
    spheres[0] = { { 0, 0, -1 }, 0.3 };
    spheres[1] = { { 1, 0, -1 }, 0.3 };
    return true;
}   

void renderer_begin_frame()
{
    camera_ray_cast(camera_handle, spheres, (b8 *)(framebuffer));

    [[maybe_unused]] bool _ = software_platform_submit_framebuffer(IMAGE_WIDTH, IMAGE_HEIGHT, 
            IMAGE_WIDTH * IMAGE_HEIGHT * BYTES_PER_PIXEL, framebuffer);
}

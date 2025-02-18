#include "warpunk.core/renderer/renderer_backend.h"

#include "warpunk.core/defines.h"
#include "warpunk.core/renderer/platform/software_platform.h"
#include "warpunk.core/renderer/camera/camera.h"

#include "warpunk.core/math/hittable.hpp"

#define BYTES_PER_PIXEL 4

static u8 framebuffer[1920 * 1080 * BYTES_PER_PIXEL];
static camera_handle_t camera_handle;
static sphere_t<f64> spheres[2];
static s32 width;
static s32 height;

namespace software_renderer
{
    [[nodiscard]] b8 renderer_startup(renderer_config_t renderer_config)
    {
        if (!software_platform_startup())
        {
            return false;
        }

        width = renderer_config.width;
        height = (s32)((f64)renderer_config.width / renderer_config.aspect_ratio);
        height = (height < 1) ? 1 : height;

        /** camera */
        camera_config_t camera_config = {
            .aspect_ratio = renderer_config.aspect_ratio,
            .focal_length = 1.0,
            .image_width = renderer_config.width,
            .viewport_height = 2.0,
            .samples_per_pixel = 16,
            .max_depth = 50,
        };
        camera_handle = camera_create(camera_config);

        /** spheres */
        spheres[0] = { { 0, 0, -1 }, 0.5 };
        spheres[1] = { { 0, -100.5, -1 }, 100 };
        return true;
    }   

    void renderer_begin_frame()
    {
        camera_ray_cast(camera_handle, spheres, framebuffer);

        [[maybe_unused]] bool _ = software_platform_submit_framebuffer(width, height, 
                width * height * BYTES_PER_PIXEL, framebuffer);
    }
}

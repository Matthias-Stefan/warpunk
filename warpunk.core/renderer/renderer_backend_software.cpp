#include "warpunk.core/renderer/renderer_backend.h"

#include "warpunk.core/defines.h"
#include "warpunk.core/renderer/platform/software_platform.h"
#include "warpunk.core/renderer/camera/camera.h"

#include "warpunk.core/math/hittable.hpp"

#define BYTES_PER_PIXEL 4

static u8 framebuffer[1920 * 1080 * BYTES_PER_PIXEL];
static camera_handle_t camera_handle;
static s32 width;
static s32 height;

static material_s<f64> metal1 = { .type = METAL, .albedo = { 0.8, 0.8, 0.8 } };
static material_s<f64> metal2 = { .type = METAL, .fuzz = 0.66, .albedo = { 0.8, 0.6, 0.2 } };
static material_s<f64> metal3 = { .type = METAL, .fuzz = 0.33, .albedo = { 0.33, 0.33, 0.33 } };
static material_s<f64> lambert1 = { .type = LAMBERT, .albedo = { 0.8, 0.8, 0.0 } };
static material_s<f64> lambert2 = { .type = LAMBERT, .albedo = { 0.1, 0.2, 0.5 } };
static material_s<f64> dielectric1 = { .type = DIELECTRIC, .refraction_index=(1.00 / 1.33) };

static sphere_s<f64> spheres[4];


namespace software_renderer
{
    b8 renderer_startup(renderer_config_s renderer_config)
    {
        if (!software_platform_startup())
        {
            return false;
        }

        width = renderer_config.width;
        height = (s32)((f64)renderer_config.width / renderer_config.aspect_ratio);
        height = (height < 1) ? 1 : height;

        /** camera */
        camera_config_s camera_config = {
            .aspect_ratio = renderer_config.aspect_ratio,
            .focal_length = 1.0,
            .image_width = renderer_config.width,
            .viewport_height = 2.0,
            .samples_per_pixel = 50,
            .max_depth = 50,
        };
        camera_handle = camera_create(camera_config);

        /** spheres */
        spheres[0] = { .center = {  0.0,    0.0, -1.2 }, .radius =   0.5, .material = &lambert2 };
        spheres[1] = { .center = { -1.0,    0.0, -1.0 }, .radius =   0.5, .material = &metal1 };
        spheres[2] = { .center = {  1.0,    0.0, -1.0 }, .radius =   0.5, .material = &dielectric1 };
        spheres[3] = { .center = {  0.0, -100.5, -1.0 }, .radius = 100.0, .material = &metal3 };

        return true;
    }   

    void renderer_begin_frame()
    {
        camera_ray_cast(camera_handle, spheres, framebuffer);

        [[maybe_unused]] bool _ = software_platform_submit_framebuffer(width, height, 
                width * height * BYTES_PER_PIXEL, framebuffer);
    }
}

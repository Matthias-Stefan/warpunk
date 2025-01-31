#include "warpunk.core/renderer/renderer_backend.h"

#include "warpunk.core/defines.h"
#include "warpunk.core/renderer/platform/software_platform.h"
#include "warpunk.core/math/v3.hpp"
#include "warpunk.core/math/ray.hpp"
#include "warpunk.core/math/hittable.hpp"

typedef struct _camera_t
{
    f64 focal_length;
    f64 viewport_height;
    f64 viewport_width;
    p3f64_t center = { 0, 0, 0 }; 
} camera_t;

#define IMAGE_WIDTH 800
#define IMAGE_HEIGHT 600
#define BYTES_PER_PIXEL 4

u8 framebuffer[IMAGE_WIDTH * IMAGE_HEIGHT * BYTES_PER_PIXEL];
camera_t camera;


v3f64_t viewport_u;
v3f64_t viewport_v;
v3f64_t pixel_delta_u;
v3f64_t pixel_delta_v;
p3f64_t viewport_upper_left;
p3f64_t pixel00_loc;

[[nodiscard]] b8 renderer_startup(renderer_config_t renderer_config)
{
    if (!software_platform_startup())
    {
        return false;
    }

    // Camera
    camera.focal_length = 1.0;
    camera.viewport_height = 2.0;
    camera.viewport_width = camera.viewport_height * static_cast<f64>(IMAGE_WIDTH) / IMAGE_HEIGHT;
    
    // Calculate the vectors across the horizontal and down the vertical viewport edges.
    viewport_u = { camera.viewport_width, 0, 0 };
    viewport_v = { 0, -camera.viewport_height, 0 };

    // Calculate the horizontal and vertical delta vectors from pixel to pixel.
    pixel_delta_u = viewport_u / IMAGE_WIDTH;
    pixel_delta_v = viewport_v / IMAGE_HEIGHT;

    // Calculate the location of the upper left pixel.
    v3f64_t z = { 0, 0, camera.focal_length };
    viewport_upper_left = camera.center - z - viewport_u/2 - viewport_v/2;
    pixel00_loc = viewport_upper_left + 0.5 * (pixel_delta_u + pixel_delta_v);

    return true;
}   

v3_t<u8> get_color_from_unit(const v3f64_t& vector)
{
    v3_t<u8> color = {}; 
    color.r = s32(255.999 * vector.r);
    color.g = s32(255.999 * vector.g);
    color.b = s32(255.999 * vector.b);
    
    return color;
}

void renderer_begin_frame()
{
    u8* row = (u8 *)framebuffer;
    for (u16 y = 0; y < IMAGE_HEIGHT; ++y)
    {
        u32* pixel = (u32 *)row;
        for (u16 x = 0; x < IMAGE_WIDTH; ++x)
        {
            auto pixel_center = pixel00_loc + (x * pixel_delta_u) + (y * pixel_delta_v);
            auto ray_dir = pixel_center - camera.center;
            rayf64_t r = { camera.center, ray_dir };
       
            v3f64_t unit_color;
            sphere_t<f64> sphere = { {0, 0, -1}, 0.5 };
            hit_record_t<f64> record = {};
            if (hit(&sphere, &r, 0, 50000, &record))
            {
                unit_color = 0.5 * v3f64_t { record.normal.x + 1, 
                                             record.normal.y + 1, 
                                             record.normal.z + 1 };
            }
            else 
            {     
                v3f64_t unit_direction = unit_vector<f64>(r.dir);
                auto a = 0.5 * (unit_direction.y + 1.0);
                unit_color = (1.0 - a) * v3f64_t{ 1.0, 1.0, 1.0 } + a * v3f64_t{ 207.0/255, 10.0/255, 44.0/255 };
            }
            
            v3_t<u8> color = get_color_from_unit(unit_color);
            
            u8 alpha = 255;
            *pixel++ =((((alpha << 24) | color.r << 16) | color.g << 8) | color.b);
        }

        row += IMAGE_WIDTH * BYTES_PER_PIXEL;
    }

    [[maybe_unused]] bool _ = software_platform_submit_framebuffer(IMAGE_WIDTH, IMAGE_HEIGHT, 
            IMAGE_WIDTH * IMAGE_HEIGHT * BYTES_PER_PIXEL, framebuffer);
}

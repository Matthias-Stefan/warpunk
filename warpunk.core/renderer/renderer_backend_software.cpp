#include "warpunk.core/renderer/renderer_backend.h"

#include "warpunk.core/defines.h"
#include "warpunk.core/renderer/platform/software_platform.h"
#include "warpunk.core/math/v3.hpp"
#include "warpunk.core/math/ray.hpp"

typedef struct _camera_t
{
    f64 focal_length;
    f64 viewport_height;
    f64 viewport_width;
    point3_t<f64> center = { 0, 0, 0 }; 
} camera_t;

u8 framebuffer[1920 * 1080 * 4];
camera_t camera;
v3f64_t viewport_u;
v3f64_t viewport_v;
v3f64_t pixel_delta_u;
v3f64_t pixel_delta_v;
point3_t<f64> viewport_upper_left;
point3_t<f64> pixel00_loc;

[[nodiscard]] b8 renderer_startup(renderer_config_t renderer_config)
{
    if (!software_platform_startup())
    {
        return false;
    }

    // Camera
    camera.focal_length = 1.0;
    camera.viewport_height = 2.0;
    camera.viewport_width = camera.viewport_height * static_cast<f64>(1920) / 1080;
    
    // Calculate the vectors across the horizontal and down the vertical viewport edges.
    viewport_u = { camera.viewport_width, 0, 0 };
    viewport_v = { 0, -camera.viewport_height, 0 };

    // Calculate the horizontal and vertical delta vectors from pixel to pixel.
    pixel_delta_u = viewport_u / 1920;
    pixel_delta_v = viewport_v / 1080;

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

f64 hit_sphere(const point3_t<f64>& center, f64 radius, const rayf64_t& ray)
{
    v3f64_t oc = center - ray.origin;
    auto a = dot(ray.dir, ray.dir);
    auto b = -2.0 * dot(ray.dir, oc);
    auto c = dot(oc, oc) - radius * radius;
    auto discriminant = b*b - 4 * a * c;
    
    if (discriminant < 0)
    {
        return -1.0;
    }
    else 
    {
        return ( -b - std::sqrt(discriminant)) / (2.0 * a);
    }
}

void renderer_begin_frame()
{
    u8* row = (u8 *)framebuffer;
    for (u16 y = 0; y < 1080; ++y)
    {
        u32* pixel = (u32 *)row;
        for (u16 x = 0; x < 1920; ++x)
        {
            auto pixel_center = pixel00_loc + (x * pixel_delta_u) + (y * pixel_delta_v);
            auto ray_dir = pixel_center - camera.center;
            rayf64_t r = { camera.center, ray_dir };
       
            v3f64_t unit_color;
            f64 t = hit_sphere(point3_t<f64>{ 0, 0, -1}, 0.5, r);
            if (t > 0.0)
            {
                v3f64_t N = unit_vector(at(r, t) - v3f64_t{ 0, 0, -1 });
                unit_color = 0.5 * v3f64_t { N.x + 1, N.y + 1, N.z + 1 };
            }
            else 
            {     
                v3f64_t unit_direction = unit_vector<f64>(r.dir);
                auto a = 0.5 * (unit_direction.y + 1.0);
                unit_color = (1.0 - a) * v3f64_t{ 1.0, 1.0, 1.0 } + a * v3f64_t{ 0.5, 0.7, 1.0 };
            }
            
            v3_t<u8> color = get_color_from_unit(unit_color);
            
            u8 alpha = 255;
            *pixel++ =((((alpha << 24) | color.r << 16) | color.g << 8) | color.b);
        }

        row += 1920 * 4;
    }

    [[maybe_unused]] bool _ = software_platform_submit_framebuffer(1920, 1080, 1920*1080*4, framebuffer);
}

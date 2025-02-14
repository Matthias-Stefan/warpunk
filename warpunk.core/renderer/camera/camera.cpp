#include "warpunk.core/renderer/camera/camera.h"

#include "warpunk.core/math/math_common.hpp"

#include "warpunk.core/math/v3.hpp"
#include "warpunk.core/math/ray.hpp"
#include "warpunk.core/math/hittable.hpp"

#define BYTES_PER_PIXEL 4

struct camera_t
{
   /** ratio of image width over height */
    f64 aspect_ratio;
    f64 focal_length;
    /** count of random samples for each pixel */
    s32 samples_per_pixel;
    /** color scale factor for a sum of pixel samples */
    f64 pixel_samples_scale;

    s32 image_width;                                
    s32 image_height;       
    p3f64_t center;

    /** location of pixel (0, 0) */
    p3f64_t pixel00_loc;    
    /** offset to pixel to the right */
    v3f64_t pixel_delta_u; 
    /** offset to pixel below */
    v3f64_t pixel_delta_v;
};

// TODO: Dynamic container!
static camera_handle_t camera_count = 0;
static camera_t cameras[10];

camera_handle_t camera_create(camera_config_t camera_config)
{
    p3f64_t center = { 0, 0, 0 };
    f64 aspect_ratio = camera_config.aspect_ratio;
    s32 image_height = (s32)((f64)camera_config.image_width / aspect_ratio);
    image_height = (image_height < 1) ? 1 : image_height;

    /** determine viewport dimensions */
    f64 viewport_height = camera_config.viewport_height;
    f64 viewport_width = viewport_height * ((f64)camera_config.image_width) / image_height;
    
    /** calculate the vectors across the horizontal and down the vertical viewport edges */
    v3f64_t viewport_u = { viewport_width, 0, 0 };
    v3f64_t viewport_v = { 0, -viewport_height, 0 };

    /** calculate the horizontal and vertical delta vectors from pixel to pixel */
    v3f64_t pixel_delta_u = viewport_u / camera_config.image_width;
    v3f64_t pixel_delta_v = viewport_v / image_height;

    /** calculate the location of the upper left pixel */
    v3f64_t z = { 0, 0, camera_config.focal_length };
    v3f64_t viewport_upper_left = center - z - (viewport_u / 2) - (viewport_v / 2);
    p3f64_t pixel00_loc = viewport_upper_left + 0.5 * (pixel_delta_u + pixel_delta_v);

    camera_handle_t camera_handle = camera_count;
    cameras[camera_count++] = {
        .aspect_ratio = aspect_ratio,
        .focal_length = camera_config.focal_length,
        .samples_per_pixel = camera_config.samples_per_pixel,
        .pixel_samples_scale = 1.0 / camera_config.samples_per_pixel,
        .image_width = camera_config.image_width,
        .image_height = image_height,
        .center = center,
        .pixel00_loc = pixel00_loc,
        .pixel_delta_u = pixel_delta_u,
        .pixel_delta_v = pixel_delta_v,
    };
    
    return camera_handle;
}

v3_t<u8> get_color_from_unit(const v3f64_t& vector)
{
    v3_t<u8> color = {};
    static const interval_t<f64> intensity = { 0.000, 0.999 };
    color.r = s32(256 * clamp(&intensity, vector.r));
    color.g = s32(256 * clamp(&intensity, vector.g));
    color.b = s32(256 * clamp(&intensity, vector.b));
    
    return color;
}

template<typename T>
[[nodiscard]] inline v3_t<T> sample_square()
{
    v3_t<T> v3 = { .x = random_f64() - 0.5,
                   .y = random_f64() - 0.5, 
                   .z = 0 };
    return v3;
}

template<typename T>
[[nodiscard]] inline ray_t<T> get_ray(camera_handle_t camera_handle, s32 x, s32 y)
{
    /** 
     * construct a camera ray originating from the origin and directed at randomly sampled
     * point around the pixel location x, y */
    camera_t* camera = &cameras[camera_handle];

    v3_t<T> offset = sample_square<T>();
    v3_t<T> pixel_sample = camera->pixel00_loc 
                         + ((x + offset.x) * camera->pixel_delta_u) 
                         + ((y + offset.y) * camera->pixel_delta_v);

    auto ray_origin = camera->center;
    auto ray_direction = pixel_sample - ray_origin;

    ray_t<T> ray = {
        .origin = ray_origin,
        .dir = ray_direction
    };

    return ray;
}

void camera_ray_cast(camera_handle_t camera_handle, void* objects, b8* out_buffer)
{
    sphere_t<f64>* spheres = (sphere_t<f64> *)objects;
    u8* row = (u8 *)out_buffer;
    camera_t* camera = &cameras[camera_handle];

    for (u16 y = 0; y < camera->image_height; ++y)
    {
        u32* pixel = (u32 *)row;
        for (u16 x = 0; x < camera->image_width; ++x)
        {
            v3f64_t unit_color = zero<f64>();
            for (int sample = 0; sample < camera->samples_per_pixel; ++sample)
            {
                rayf64_t ray = get_ray<f64>(camera_handle, x, y); 

                bool sphere_hit = false;
                for (int sphere_idx = 0; sphere_idx < 2; ++sphere_idx)
                {
                    sphere_t<f64>* sphere = &spheres[sphere_idx];
                    hit_record_t<f64> record = {};
                    sphere_hit = hit(sphere, &ray, { 0, inf64 }, &record);
                    if (sphere_hit)
                    {
                        unit_color += 0.5 * v3f64_t { record.normal.x + 1, 
                                                      record.normal.y + 1, 
                                                      record.normal.z + 1 };
                        break;
                    }
                }

                if (!sphere_hit)
                {
                    v3f64_t unit_direction = unit_vector<f64>(ray.dir);
                    auto a = 0.5 * (unit_direction.y + 1.0);
                    unit_color += (1.0 - a) * v3f64_t{ 1.0, 1.0, 1.0 } + a * v3f64_t{ 207.0/255, 10.0/255, 44.0/255 };
                }
            }
            
            v3_t<u8> color = get_color_from_unit(unit_color * camera->pixel_samples_scale);
            
            u8 alpha = 255;
            *pixel++ =((((alpha << 24) | color.r << 16) | color.g << 8) | color.b);
        }

        row += camera->image_width * BYTES_PER_PIXEL;
    }
}



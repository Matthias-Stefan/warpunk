#include "warpunk.core/src/renderer/camera/camera.h"

#include "warpunk.core/src/math/math_common.hpp"
#include "warpunk.core/src/math/v3.hpp"
#include "warpunk.core/src/math/ray.hpp"
#include "warpunk.core/src/math/hittable.hpp"
#include "warpunk.core/src/renderer/materials/material.hpp"

#include "warpunk.core/src/platform/platform.h"

#define BYTES_PER_PIXEL 4

struct camera
{
   /** ratio of image width over height */
    f64 aspect_ratio;
    f64 focal_length;
    /** count of random samples for each pixel */
    s32 samples_per_pixel;
    /** color scale factor for a sum of pixel samples */
    f64 pixel_samples_scale;
    /** maximum number of ray bounces into scene */
    s32 max_depth;

    s32 image_width;                                
    s32 image_height;       
    p3f64 center;

    /** location of pixel (0, 0) */
    p3f64 pixel00_loc;    
    /** offset to pixel to the right */
    v3f64 pixel_delta_u; 
    /** offset to pixel below */
    v3f64 pixel_delta_v;
};

// TODO: Dynamic container!
static camera_handle camera_count = 0;
static camera cameras[10];

camera_handle camera_create(camera_config camera_config)
{
    p3f64 center = { 0, 0, 0 };
    f64 aspect_ratio = camera_config.aspect_ratio;
    s32 image_height = (s32)((f64)camera_config.image_width / aspect_ratio);
    image_height = (image_height < 1) ? 1 : image_height;

    /** determine viewport dimensions */
    f64 viewport_height = camera_config.viewport_height;
    f64 viewport_width = viewport_height * ((f64)camera_config.image_width) / image_height;
    
    /** calculate the vectors across the horizontal and down the vertical viewport edges */
    v3f64 viewport_u = { viewport_width, 0, 0 };
    v3f64 viewport_v = { 0, -viewport_height, 0 };

    /** calculate the horizontal and vertical delta vectors from pixel to pixel */
    v3f64 pixel_delta_u = viewport_u / camera_config.image_width;
    v3f64 pixel_delta_v = viewport_v / image_height;

    /** calculate the location of the upper left pixel */
    v3f64 z = { 0, 0, camera_config.focal_length };
    v3f64 viewport_upper_left = center - z - (viewport_u / 2) - (viewport_v / 2);
    p3f64 pixel00_loc = viewport_upper_left + 0.5 * (pixel_delta_u + pixel_delta_v);

    camera_handle camera_handle = camera_count;
    cameras[camera_count++] = {
        .aspect_ratio = aspect_ratio,
        .focal_length = camera_config.focal_length,
        .samples_per_pixel = camera_config.samples_per_pixel,
        .pixel_samples_scale = 1.0 / camera_config.samples_per_pixel,
        .max_depth = camera_config.max_depth,
        .image_width = camera_config.image_width,
        .image_height = image_height,
        .center = center,
        .pixel00_loc = pixel00_loc,
        .pixel_delta_u = pixel_delta_u,
        .pixel_delta_v = pixel_delta_v,
    };
    
    return camera_handle;
}

inline f64 linear_to_gamma(f64 linear_component)
{
    if (linear_component > 0)
    {
        return std::sqrt(linear_component);
    }

    return 0.0;
}

v3<u8> get_color_from_unit(const v3f64& vector)
{
    v3<u8> color = {};
    static const interval<f64> intensity = { 0.000, 0.999 };
    color.r = s32(256 * clamp(&intensity, linear_to_gamma(vector.r)));
    color.g = s32(256 * clamp(&intensity, linear_to_gamma(vector.g)));
    color.b = s32(256 * clamp(&intensity, linear_to_gamma(vector.b)));
    
    return color;
}

template<typename T>
inline v3<T> sample_square()
{
    v3<T> v3 = { .x = randreal01<f64>() - 0.5,
                   .y = randreal01<f64>() - 0.5, 
                   .z = 0 };
    return v3;
}

template<typename T>
inline ray<T> get_ray(camera_handle camera_handle, s32 x, s32 y)
{
    /** 
     * construct a camera ray originating from the origin and directed at randomly sampled
     * point around the pixel location x, y */
    camera* camera = &cameras[camera_handle];

    v3<T> offset = sample_square<T>();
    v3<T> pixel_sample = camera->pixel00_loc 
                         + ((x + offset.x) * camera->pixel_delta_u) 
                         + ((y + offset.y) * camera->pixel_delta_v);

    auto ray_origin = camera->center;
    auto ray_direction = pixel_sample - ray_origin;

    ray<T> ray = {
        .origin = ray_origin,
        .dir = ray_direction
    };

    return ray;
}

template<typename T>
v3f64 ray_color(ray<T>* r, sphere<T>* spheres, s32 depth)
{
    if (depth <= 0)
    {
        return v3f64 { .r = 0.0, 
                         .g = 0.0, 
                         .b = 0.0 };
    }

    hit_record<T> record = {};
    bool hit_sphere = false;
    for (int sphere_idx = 0; sphere_idx < 4; ++sphere_idx)
    {
        sphere<f64>* sphere = &spheres[sphere_idx];
        if (hit(sphere, r, { 0.001, inf64 }, &record))
        {
            hit_sphere = true;
            break;
        }
    }

    if (hit_sphere)
    {
        ray<T> scattered;
        v3<T> attenuation = zero<T>();
        if (scatter(record.material, r, &record, &attenuation, &scattered))
        {
            return attenuation * ray_color(&scattered, spheres, depth-1); 
        }
        return v3f64 { 0.0, 0.0, 0.0 };
    }


    v3f64 unit_direction = unit_vector<T>(r->dir);
    auto a = 0.5 * (unit_direction.y + 1.0);
    return (1.0 - a) * v3f64{ 1.0, 1.0, 1.0 } + a * v3f64{ 0.5, 0.7, 1.0 };
 
}

typedef struct render_chunk
{
    camera_handle camera_handle;
    void* objects;
    u8* out_buffer;
    u16 x_start;
    u16 y_start;
    u16 width;
    u16 height;
} render_chunk;

void camera_ray_cast_chunk(void* data)
{
    render_chunk* chunk = (render_chunk *)data;

    sphere<f64>* spheres = (sphere<f64> *)chunk->objects;
    u8* row = chunk->out_buffer;
    camera* camera = &cameras[chunk->camera_handle];

    for (u16 y = chunk->y_start; y < chunk->height; ++y)
    {
        u32* pixel = (u32 *)row;
        for (u16 x = chunk->x_start; x < chunk->width; ++x)
        {
            v3f64 unit_color = zero<f64>();
            for (int sample = 0; sample < camera->samples_per_pixel; ++sample)
            {
                rayf64 ray = get_ray<f64>(chunk->camera_handle, x, y);
                unit_color += ray_color<f64>(&ray, spheres, camera->max_depth);
            }

            v3<u8> color = get_color_from_unit(unit_color * camera->pixel_samples_scale);
            
            u8 alpha = 255;
            *pixel++ =((((alpha << 24) | color.r << 16) | color.g << 8) | color.b);
        }

        row += camera->image_width * BYTES_PER_PIXEL;
    }
}

void camera_ray_cast(camera_handle camera_handle, void* objects, u8* out_buffer)
{
    camera* camera = &cameras[camera_handle];

    s32 chunk_width = camera->image_width / 4;
    s32 chunk_height = camera->image_height / 4;

    render_chunk render_chunk[16];
    for (int y = 0; y < 4; ++y)
    {
        for (int x = 0; x < 4; ++x)
        {
            int x_start = chunk_width * x;
            int y_start = chunk_height * y; 

            int offset = x + y * 4;
            render_chunk[offset].camera_handle = camera_handle;
            render_chunk[offset].objects = objects;
            render_chunk[offset].out_buffer = out_buffer + (x * chunk_width * BYTES_PER_PIXEL) + (y * chunk_height * camera->image_width * BYTES_PER_PIXEL);
            render_chunk[offset].x_start = x_start;
            render_chunk[offset].y_start = y_start;
            render_chunk[offset].width = x_start + chunk_width;
            render_chunk[offset].height = y_start + chunk_height;
        }
    }

    platform_threading_job job = {};
    job.function = camera_ray_cast_chunk;
    job.arg = render_chunk;
    job.arg_size = sizeof(render_chunk);

    thread_ticket ticket;
    platform_threadpool_add(&job, 16, &ticket);
    platform_threadpool_sync(ticket, 0);
}

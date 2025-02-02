#include "warpunk.core/renderer/camera/camera.h"

#include <vector>

#include "warpunk.core/math/math_common.hpp"
#include "warpunk.core/math/v3.hpp"
#include "warpunk.core/math/ray.hpp"
#include "warpunk.core/math/hittable.hpp"

struct camera_t
{
   /** ratio of image width over height */
    f64 aspect_ratio;
    f64 focal_length;
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

std::vector<camera_t> cameras;

camera_t* camera_create(camera_config_t camera_config)
{
    p3f64_t center = { 0, 0, 0 };
    f64 aspect_ratio = 1.0;
    s32 image_width = (s32)(camera_config.image_width / aspect_ratio);
    s32 image_height = (camera_config.image_height < 1) ? 1 : image_height;

    /** determine viewport dimensions */
    f64 viewport_height = camera_config.viewport_height;
    f64 viewport_width = viewport_height * ((f64)image_width) / image_height;
    
    /** calculate the vectors across the horizontal and down the vertical viewport edges */
    v3f64_t viewport_u = { viewport_width, 0, 0 };
    v3f64_t viewport_v = { 0, -viewport_height, 0 };

    /** calculate the horizontal and vertical delta vectors from pixel to pixel */
    v3f64_t pixel_delta_u = viewport_u / image_width;
    v3f64_t pixel_delta_v = viewport_v / image_height;

    /** calculate the location of the upper left pixel */
    v3f64_t z = { 0, 0, camera_config.focal_length };
    v3f64_t viewport_upper_left = center - z - (viewport_u / 2) - (viewport_v / 2);
    p3f64_t pixel00_loc = viewport_upper_left + 0.5 * (pixel_delta_u + pixel_delta_v);

    camera_t camera = {
        .aspect_ratio = aspect_ratio,
        .focal_length = camera_config.focal_length,
        .image_width = image_width ,
        .image_height = image_height,
        .center = center,
        .pixel00_loc = pixel00_loc,
        .pixel_delta_u = pixel_delta_u,
        .pixel_delta_v = pixel_delta_v,

    };

    cameras.emplace_back(camera);
    
    return &cameras.back();
}

void camera_ray_cast(const camera_t* camera, void* objects)
{

}

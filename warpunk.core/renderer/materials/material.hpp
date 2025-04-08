#pragma once

#include "warpunk.core/defines.h"
#include "warpunk.core/math/v3.hpp"
#include "warpunk.core/math/ray.hpp"

template<typename T>
struct hit_record_t;

enum material_type_t
{
    LAMBERT,
    METAL,
    DIELECTRIC,
};

template<typename T>
struct material_t 
{
    material_type_t type;    
    T fuzz {};
    v3_t<T> albedo;
    f64 refraction_index;
};

template<typename T>
inline b8 scatter(material_t<T>* material, 
        ray_t<T>* ray, hit_record_t<T>* record, 
        v3_t<T>* out_attenuation, ray_t<T>* out_scattered)
{
    if (material)
    {
        switch (material->type)
        {
            case LAMBERT:
            {
                auto scatter_direction = record->normal + random_unit_vector<T>();
                
                /** catch degenerate scatter direction */
                if (near_zero(scatter_direction))
                {
                    scatter_direction = record->normal;
                }

                *out_scattered = ray_t<T> { record->pos, scatter_direction };
                *out_attenuation = material->albedo;
                return true;
            }

            case METAL:
            {
                auto reflected = reflect(ray->dir, record->normal);
                reflected = unit_vector(reflected) + (material->fuzz * random_unit_vector<T>());
                *out_scattered = ray_t<T> { record->pos, reflected };
                *out_attenuation = material->albedo;
                return (dot(out_scattered->dir, record->normal) > 0);
            }
            case DIELECTRIC:
            {
                *out_attenuation = { 1.0, 1.0, 1.0 };
                f64 ri = record->front_face ? (1.0 / material->refraction_index) : material->refraction_index; 
                
                v3_t<T> unit_dir = unit_vector(ray->dir);
                f64 cos_theta = std::fmin(dot(-unit_dir, record->normal), 1.0);
                f64 sin_theta = std::sqrt(1.0 - cos_theta * cos_theta);

                b8 cannot_refract = ri * sin_theta > 1.0;
                v3_t<T> direction;
                if (cannot_refract)
                {
                    direction = reflect(unit_dir, record->normal);
                }
                else 
                {
                    direction = refract(unit_dir, record->normal, ri);
                }

                *out_scattered = ray_t<T> { record->pos, direction };
                return true;
            }
        }
    }

    return false;
}


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
};

template<typename T>
struct material_t 
{
    material_type_t type;    
    T fuzz {};
    v3_t<T> albedo;

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
        }
    }

    return false;
}


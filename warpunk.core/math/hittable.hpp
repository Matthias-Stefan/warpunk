#pragma once

#include "warpunk.core/math/math_common.hpp"
#include "warpunk.core/math/v3.hpp"
#include "warpunk.core/math/ray.hpp"
#include "warpunk.core/renderer/materials/material.hpp"

#include <type_traits>

template<typename T>
struct hit_record_t
{
    p3_t<T> pos;
    v3_t<T> normal;
    f64 t;
    b8 front_face;
    material_t<T>* material;
};


template<typename T>
struct sphere_t
{
    p3_t<T> center;
    f64 radius;
    material_t<T>* material;
};

/** 
 * Sets the hit record normal vector 
 * NOTE: the parameter `outward_normal` is assumed to have unit length 
 */
template<typename T>
inline void set_face_normal(hit_record_t<T>* hit_record, const ray_t<T>* ray, const v3_t<T>* outward_normal)
{
    hit_record->front_face = dot(ray->dir, *outward_normal) < 0;
    hit_record->normal = hit_record->front_face ? *outward_normal : -(*outward_normal);
}

template<typename S, typename T, typename std::enable_if<!std::is_same<S, sphere_t<T>>::value>::type* = nullptr>
[[nodiscard]] inline b8 hit(const S* object, ray_t<T>* ray, interval_t<T> interval, hit_record_t<T>* out_hit_record) = delete;

/** */
template<typename S, typename T, typename std::enable_if<std::is_same<S, sphere_t<T>>::value>::type* = nullptr>
[[nodiscard]] inline b8 hit(const S* sphere, ray_t<T>* ray, interval_t<T> interval, hit_record_t<T>* out_hit_record)
{
    v3f64_t oc = sphere->center - ray->origin;
    auto a = length_squared(ray->dir);
    auto h = dot(ray->dir, oc);
    auto c = length_squared(oc) - sphere->radius * sphere->radius;
    auto discriminant = (h * h) - (a * c);
    if (discriminant < 0)
    {
        return false;
    }

    auto sqrtd = std::sqrt(discriminant);

    if (a == 0.0)
    {
        return false;
    }

    // find the nearest root that lies in the acceptable range.
    auto root = (h - sqrtd) / a;
    if (!surrounds(&interval, root))
    {
        root = (h + sqrtd) / a;
        if (!surrounds(&interval, root))
        {
            return false;
        }
    }

    out_hit_record->t = root;
    out_hit_record->pos = at(ray, out_hit_record->t);
    v3_t<T> outward_normal = (out_hit_record->pos - sphere->center) / sphere->radius;
    set_face_normal(out_hit_record, ray, &outward_normal);
    out_hit_record->material = sphere->material;

    return true;
}







#pragma once

#include "warpunk.core/src/math/math_common.hpp"
#include "warpunk.core/src/math/v3.hpp"
#include "warpunk.core/src/math/ray.hpp"
#include "warpunk.core/src/renderer/materials/material.hpp"

#include <type_traits>

template<typename T>
struct hit_record
{
    p3<T> pos;
    v3<T> normal;
    f64 t;
    b8 front_face;
    material<T>* material;
};


template<typename T>
struct sphere
{
    p3<T> center;
    f64 radius;
    material<T>* material;
};

/** 
 * Sets the hit record normal vector 
 * NOTE: the parameter `outward_normal` is assumed to have unit length 
 */
template<typename T>
inline void set_face_normal(hit_record<T>* hit_record, const ray<T>* ray, const v3<T>* outward_normal)
{
    hit_record->front_face = dot(ray->dir, *outward_normal) < 0;
    hit_record->normal = hit_record->front_face ? *outward_normal : -(*outward_normal);
}

template<typename S, typename T, typename std::enable_if<!std::is_same<S, sphere<T>>::value>::type* = nullptr>
inline b8 hit(const S* object, ray<T>* ray, interval<T> interval, hit_record<T>* out_hit_record) = delete;

/** */
template<typename S, typename T, typename std::enable_if<std::is_same<S, sphere<T>>::value>::type* = nullptr>
inline b8 hit(const S* sphere, ray<T>* ray, interval<T> interval, hit_record<T>* out_hit_record)
{
    v3f64 oc = sphere->center - ray->origin;
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
    v3<T> outward_normal = (out_hit_record->pos - sphere->center) / sphere->radius;
    set_face_normal(out_hit_record, ray, &outward_normal);
    out_hit_record->material = sphere->material;

    return true;
}







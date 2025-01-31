#pragma once

#include "warpunk.core/defines.h"
#include "warpunk.core/math/v3.hpp"

template<typename T>
struct ray_t
{
    p3_t<T> origin;
    v3_t<T> dir;
};

template<typename T>
p3_t<T> at(const ray_t<T>* ray, f64 t) 
{
    return ray->origin + t * ray->dir;
}

using rayf32_t = ray_t<f32>;
using rayf64_t = ray_t<f64>;

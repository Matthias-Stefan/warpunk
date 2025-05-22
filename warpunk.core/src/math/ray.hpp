#pragma once

#include "warpunk.core/src/defines.h"
#include "warpunk.core/src/math/v3.hpp"

template<typename T>
struct ray_s
{
    p3_s<T> origin;
    v3_s<T> dir;
};

template<typename T>
p3_s<T> at(const ray_s<T>* ray, f64 t) 
{
    return ray->origin + t * ray->dir;
}

using rayf32_s = ray_s<f32>;
using rayf64_s = ray_s<f64>;

#pragma once

#include "warpunk.core/src/defines.h"
#include "warpunk.core/src/math/v3.hpp"

template<typename T>
struct ray
{
    p3<T> origin;
    v3<T> dir;
};

template<typename T>
p3<T> at(const ray<T>* ray, f64 t) 
{
    return ray->origin + t * ray->dir;
}

using rayf32 = ray<f32>;
using rayf64 = ray<f64>;

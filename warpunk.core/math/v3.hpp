#pragma once

#include "warpunk.core/defines.h"

#include <cmath>

template<typename T>
struct v3_t
{
    union
    {
        struct
        {
            T x;
            T y;
            T z;
        };
        struct
        {
            T r;
            T g;
            T b;
        };
    };
};

template<typename T>
v3_t<T> operator-(const v3_t<T>& vector)
{
    return v3_t<T> { .x = -vector.x,
                     .y = -vector.y,
                     .z = -vector.z };
}

template<typename T>
v3_t<T> operator+(const v3_t<T>& v1, const v3_t<T>& v2)
{
    return v3_t<T> { .x = v1.x + v2.x,
                     .y = v1.y + v2.y, 
                     .z = v1.z + v2.z }; 
}

template<typename T>
v3_t<T>& operator+=(v3_t<T>& v1, const v3_t<T>& v2)
{
    v1.x += v2.x;
    v1.y += v2.y;
    v1.z += v2.z;
    return v1;
}

template<typename T>
v3_t<T> operator*(const v3_t<T>& v1, const v3_t<T>& v2)
{
    return v3_t<T> { .x = v1.x * v2.x,
                     .y = v1.y * v2.y, 
                     .z = v1.z * v2.z }; 
}

template<typename T>
v3_t<T>& operator*=(v3_t<T>& v1, const v3_t<T>& v2)
{
    v1.x *= v2.x;
    v1.y *= v2.y;
    v1.z *= v2.z;
    return v1;
}

template<typename T>
v3_t<T>& operator/=(v3_t<T>& v1, T value)
{
    return v1 *= 1.0 / value;
}

template<typename T>
T length_squared(const v3_t<T>& vector)
{
    return vector.x * vector.x + 
           vector.y * vector.y + 
           vector.z * vector.z;
}

template<typename T>
T length(const v3_t<T>& vector)
{
    return std::sqrt(length_squared(vector));
}



template<typename T>
using point3_t = v3_t<T>;

using v3f32_t = v3_t<f32>;
using v3f64_t = v3_t<f64>;

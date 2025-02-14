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
v3_t<T> operator-(const v3_t<T>& v1, const v3_t<T>& v2)
{
    return v3_t<T> { .x = v1.x - v2.x,
                     .y = v1.y - v2.y, 
                     .z = v1.z - v2.z }; 
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
v3_t<T> operator*(f64 t, const v3_t<T>& vector)
{
    return v3_t<T> { .x = t * vector.x,
                     .y = t * vector.y, 
                     .z = t * vector.z }; 
}

template<typename T>
v3_t<T> operator*(const v3_t<T>& vector, f64 t)
{
    return v3_t<T> { .x = vector.x * t,
                     .y = vector.y * t, 
                     .z = vector.z * t }; 
}

template<typename T>
v3_t<T>& operator/=(v3_t<T>& v1, T value)
{
    return v1 *= 1.0 / value;
}

template<typename T>
v3_t<T> operator/(const v3_t<T>& vector, f64 t)
{
    return (1/t) * vector;
}

template<typename T>
[[nodiscard]] T length_squared(const v3_t<T>& vector)
{
    return vector.x * vector.x + 
           vector.y * vector.y + 
           vector.z * vector.z;
}

template<typename T>
[[nodiscard]] T length(const v3_t<T>& vector)
{
    return std::sqrt(length_squared(vector));
}

template<typename T>
[[nodiscard]] f64 dot(const v3_t<T>& v1, const v3_t<T>& v2)
{
    return v1.x * v2.x +
           v1.y * v2.y +
           v1.z * v2.z;
}

template<typename T>
[[nodiscard]] v3_t<T> cross(const v3_t<T>& v1, const v3_t<T>& v2)
{
    return v3_t<T> { .x = v1.y * v2.z - v1.z * v2.y,
                     .y = v1.z * v2.x - v1.x * v2.z,
                     .z = v1.x * v2.y - v1.y * v2.x };
}

template<typename T>
[[nodiscard]] v3_t<T> unit_vector(const v3_t<T>& vector)
{
    return vector / length(vector);
}

template<typename T> [[nodiscard]] inline v3_t<T> zero() = delete;
template<typename T, typename std::enable_if_t<
    std::is_same_v<T, s8> ||
    std::is_same_v<T, s16> ||
    std::is_same_v<T, s32> ||
    std::is_same_v<T, s64>, int> = 0>
[[nodiscard]] inline v3_t<T> zero()
{
    return v3_t<T> { .x = 0, 
                     .y = 0,
                     .z = 0 };
}
template<>
[[nodiscard]] inline v3_t<f32> zero()
{
    return v3_t<f32> { .x = 0.0f,
                       .y = 0.0f,
                       .z = 0.0f };
}
template<>
[[nodiscard]] inline v3_t<f64> zero()
{
    return v3_t<f64> { .x = 0.0,
                       .y = 0.0,
                       .z = 0.0 };
}


template<typename T>
using p3_t = v3_t<T>;

using v3f32_t = v3_t<f32>;
using v3f64_t = v3_t<f64>;

using p3f32_t = p3_t<f32>;
using p3f64_t = p3_t<f64>;

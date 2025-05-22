#pragma once

#include "warpunk.core/src/defines.h"
#include "warpunk.core/src/math/math_common.hpp"

#include <cmath>

template<typename T>
struct v3_s
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
v3_s<T> operator-(const v3_s<T>& vector)
{
    return v3_s<T> { .x = -vector.x,
                     .y = -vector.y,
                     .z = -vector.z };
}

template<typename T>
v3_s<T> operator+(const v3_s<T>& v1, const v3_s<T>& v2)
{
    return v3_s<T> { .x = v1.x + v2.x,
                     .y = v1.y + v2.y, 
                     .z = v1.z + v2.z }; 
}

template<typename T>
v3_s<T>& operator+=(v3_s<T>& v1, const v3_s<T>& v2)
{
    v1.x += v2.x;
    v1.y += v2.y;
    v1.z += v2.z;
    return v1;
}

template<typename T>
v3_s<T> operator-(const v3_s<T>& v1, const v3_s<T>& v2)
{
    return v3_s<T> { .x = v1.x - v2.x,
                     .y = v1.y - v2.y, 
                     .z = v1.z - v2.z }; 
}


template<typename T>
v3_s<T> operator*(const v3_s<T>& v1, const v3_s<T>& v2)
{
    return v3_s<T> { .x = v1.x * v2.x,
                     .y = v1.y * v2.y, 
                     .z = v1.z * v2.z }; 
}

template<typename T>
v3_s<T>& operator*=(v3_s<T>& v1, const v3_s<T>& v2)
{
    v1.x *= v2.x;
    v1.y *= v2.y;
    v1.z *= v2.z;
    return v1;
}

template<typename T>
v3_s<T> operator*(f64 t, const v3_s<T>& vector)
{
    return v3_s<T> { .x = t * vector.x,
                     .y = t * vector.y, 
                     .z = t * vector.z }; 
}

template<typename T>
v3_s<T> operator*(const v3_s<T>& vector, f64 t)
{
    return v3_s<T> { .x = vector.x * t,
                     .y = vector.y * t, 
                     .z = vector.z * t }; 
}

template<typename T>
v3_s<T>& operator/=(v3_s<T>& v1, T value)
{
    return v1 *= 1.0 / value;
}

template<typename T>
v3_s<T> operator/(const v3_s<T>& vector, f64 t)
{
    return (1/t) * vector;
}

template<typename T>
T length_squared(const v3_s<T>& vector)
{
    return vector.x * vector.x + 
           vector.y * vector.y + 
           vector.z * vector.z;
}

template<typename T>
T length(const v3_s<T>& vector)
{
    return std::sqrt(length_squared(vector));
}

template<typename T>
f64 dot(const v3_s<T>& v1, const v3_s<T>& v2)
{
    return v1.x * v2.x +
           v1.y * v2.y +
           v1.z * v2.z;
}

template<typename T>
v3_s<T> cross(const v3_s<T>& v1, const v3_s<T>& v2)
{
    return v3_s<T> { .x = v1.y * v2.z - v1.z * v2.y,
                     .y = v1.z * v2.x - v1.x * v2.z,
                     .z = v1.x * v2.y - v1.y * v2.x };
}

template<typename T> 
inline v3_s<T> zero() = delete;

template<typename T, typename std::enable_if_t<
    std::is_same_v<T, s8> ||
    std::is_same_v<T, s16> ||
    std::is_same_v<T, s32> ||
    std::is_same_v<T, s64>, int> = 0>
inline v3_s<T> zero()
{
    return v3_s<T> { .x = 0, 
                     .y = 0,
                     .z = 0 };
}

template<>
inline v3_s<f32> zero()
{
    return v3_s<f32> { .x = 0.0f,
                       .y = 0.0f,
                       .z = 0.0f };
}

template<>
inline v3_s<f64> zero()
{
    return v3_s<f64> { .x = 0.0,
                       .y = 0.0,
                       .z = 0.0 };
}

template<typename T> 
inline v3_s<T> random_vector()
{
    if constexpr (std::is_same_v<T, f32> || std::is_same_v<T, f64>)
    {
        return v3_s<T> { .x = randreal<T>(),
                         .y = randreal<T>(),
                         .z = randreal<T>() };
    }
    else if constexpr (std::is_same_v<T, s16> || std::is_same_v<T, s32> || std::is_same_v<T, s64>)
    {
        return v3_s<T> { .x = randint<T>(),
                         .y = randint<T>(),
                         .z = randint<T>() };

    }

    return v3_s<T> { 0, 0, 0 };
}

template<typename T> 
inline v3_s<T> random_vector(T low, T high)
{
    if constexpr (std::is_same_v<T, f32> || std::is_same_v<T, f64>)
    {
        return v3_s<T> { .x = randreal<T>(low, high),
                         .y = randreal<T>(low, high),
                         .z = randreal<T>(low, high) };
    }
    else if constexpr (std::is_same_v<T, s16> || std::is_same_v<T, s32> || std::is_same_v<T, s64>)
    {
        return v3_s<T> { .x = randint<T>(low, high),
                         .y = randint<T>(low, high),
                         .z = randint<T>(low, high) };

    }

    return v3_s<T> { 0, 0, 0 };
}

template<typename T> 
inline v3_s<T> random_vector01()
{
    if constexpr (std::is_same_v<T, f32> || std::is_same_v<T, f64>)
    {
        return v3_s<T> { .x = randreal01<T>(),
                         .y = randreal01<T>(),
                         .z = randreal01<T>() };
    }

    return v3_s<T> { 0, 0, 0 };
}

template<typename T>
inline v3_s<T> unit_vector(const v3_s<T>& vector)
{
    return vector / length(vector);
}

template<typename T>
inline v3_s<T> random_unit_vector()
{
    while (true)
    {
        v3_s<T> p = random_vector01<T>();
        auto lensq = length_squared(p);
        if (1e-160 < lensq && lensq <= 1)
        {
            return p / std::sqrt(lensq);
        }
    }
}

template<typename T>
inline v3_s<T> random_on_hemisphere(const v3_s<T> normal)
{
    v3_s<T> on_unit_sphere = random_unit_vector<T>();
    if (dot(on_unit_sphere, normal) > 0)
    {
        return on_unit_sphere;
    }
    else
    {
        return -on_unit_sphere;
    }
}

template<typename T>
b8 near_zero(const v3_s<T>& vector)
{
    auto s = 1e-8;
    return (std::fabs(vector.x) < s) && (std::fabs(vector.y) < s) && (std::fabs(vector.z) < s);
}

template<typename T>
v3_s<T> reflect(const v3_s<T>& v1, const v3_s<T>& v2)
{
    return v1 - 2 * dot(v1, v2) * v2;
}

template<typename T>
v3_s<T> refract(const v3_s<T>& uv, const v3_s<T>& n, f64 etai_over_etat)
{
    auto cos_theta = std::fmin(dot(-uv, n), 1.0);
    v3_s<T> r_out_perp = etai_over_etat * (uv + cos_theta * n);
    v3_s<T> r_out_parallel = -std::sqrt(std::fabs(1.0 - length_squared(r_out_perp))) * n;
    return r_out_perp + r_out_parallel;
}

template<typename T>
using p3_s = v3_s<T>;

using v3f32_s = v3_s<f32>;
using v3f64_s = v3_s<f64>;

using p3f32_s = p3_s<f32>;
using p3f64_s = p3_s<f64>;

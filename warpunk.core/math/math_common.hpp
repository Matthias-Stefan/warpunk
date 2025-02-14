#pragma once

#include "warpunk.core/defines.h"

#include <cmath>
#include <cstdlib>
#include <limits>
#include <type_traits>

const f32 inf32 = std::numeric_limits<f32>::infinity();
const f64 inf64 = std::numeric_limits<f64>::infinity();

const f32 pi32 = 3.14159265358979323846f;
const f64 pi64 = 3.14159265358979323846;

// degrees & radians

/** */
template<typename T>
[[nodiscard]] inline T degrees_to_radians(T degrees) = delete;

/** */
template<typename T, std::enable_if_t<std::is_same_v<T, f32> || std::is_same_v<T, f64>, int> = 0>
[[nodiscard]] inline T degrees_to_radians(T degrees) 
{
    if constexpr (std::is_same_v<T, f32>)
    {
        return degrees * (pi32 / 180.0f);
    }
    else
    {
        return degrees * (pi64 / 180.0);
    }
}

// interval

template<typename T>
struct interval_t
{
    T min;
    T max;
};

/** */
template<typename T>
[[nodiscard]] constexpr const interval_t<T> get_universe_interval()
{
    return interval_t<T> { .min = -std::numeric_limits<T>::infinity(),
                           .max = +std::numeric_limits<T>::infinity() };
}

/** */
template<typename T>
[[nodiscard]] constexpr const interval_t<T> get_empty_interval()
{
    return interval_t<T> { .min = +std::numeric_limits<T>::infinity(),
                           .max = -std::numeric_limits<T>::infinity() };
}

/** */
template<typename T>
[[nodiscard]] T range(const interval_t<T>* interval)
{
    return interval->max - interval->min;
}

/** */
template<typename T>
[[nodiscard]] T contains(const interval_t<T>* interval, T value)
{
    return interval->min <= value && value <= interval->max;
}

/** */
template<typename T>
[[nodiscard]] T surrounds(const interval_t<T>* interval, T value)
{
    return interval->min < value && value < interval->max;
}

/** */
template<typename T>
[[nodiscard]] T clamp(const interval_t<T>* interval, T value)
{
    if (value < interval->min)
    {
        return interval->min;
    }
    if (value > interval->max)
    {
        return interval->max;
    }
    return value;
}

// random

/** */
[[nodiscard]] inline f64 random_f64()
{
    //* returns a random real in [0, 1) */
    return std::rand() / (RAND_MAX + 1.0);
}

/** */
[[nodiscard]] inline f64 random_f64(f64 low, f64 high)
{
    return low + (high - low) * random_f64();
}







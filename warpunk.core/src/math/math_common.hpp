#pragma once

#include "warpunk.core/src/defines.h"

#include <cmath>
#include <limits>
#include <random>
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
struct interval
{
    T min;
    T max;
};

/** */
template<typename T>
[[nodiscard]] constexpr const interval<T> get_universe_interval()
{
    return interval<T> { .min = -std::numeric_limits<T>::infinity(),
                         .max = +std::numeric_limits<T>::infinity() };
}

/** */
template<typename T>
[[nodiscard]] constexpr const interval<T> get_empty_interval()
{
    return interval<T> { .min = +std::numeric_limits<T>::infinity(),
                         .max = -std::numeric_limits<T>::infinity() };
}

/** */
template<typename T>
[[nodiscard]] T range(const interval<T>* interval)
{
    return interval->max - interval->min;
}

/** */
template<typename T>
[[nodiscard]] T contains(const interval<T>* interval, T value)
{
    return interval->min <= value && value <= interval->max;
}

/** */
template<typename T>
[[nodiscard]] T surrounds(const interval<T>* interval, T value)
{
    return interval->min < value && value < interval->max;
}

template<typename  T>
[[nodiscard]] T clamp(T min, T max, T value)
{
    if (value < min)
    {
        return min;
    }
    if (value > max)
    {
        return max;
    }
    return value;
}

/** */
template<typename T>
[[nodiscard]] T clamp(const interval<T>* interval, T value)
{
    return clamp<T>(interval->min, interval->max, value);
}

// random
static std::random_device random_device;
static std::mt19937 gen(random_device()); 

/** */
template<typename T, std::enable_if_t<std::is_same_v<T, s16> || std::is_same_v<T, s32> || std::is_same_v<T, s64>, int> = 0>
[[nodiscard]] inline T randint()
{
    std::uniform_int_distribution<T> distribution(std::numeric_limits<T>::min(), std::numeric_limits<T>::max());
    return distribution(gen);
}

/** */
template<typename T, std::enable_if_t<std::is_same_v<T, s16> || std::is_same_v<T, s32> || std::is_same_v<T, s64>, int> = 0>
[[nodiscard]] inline T randint(T low, T high)
{
    std::uniform_int_distribution<T> distribution(low, high);
    return distribution(gen);
}

/** */
template<typename T, std::enable_if_t<std::is_same_v<T, f32> || std::is_same_v<T, f64>, int> = 0>
[[nodiscard]] inline T randreal()
{
    std::uniform_real_distribution<T> distribution(std::numeric_limits<T>::min(), std::numeric_limits<T>::max());
    return distribution(gen);
}

/** */
template<typename T, std::enable_if_t<std::is_same_v<T, f32> || std::is_same_v<T, f64>, int> = 0>
[[nodiscard]] inline T randreal(T low, T high)
{
    std::uniform_real_distribution<T> distribution(low, high);
    return distribution(gen);
}

template<typename T>
[[nodiscard]] inline T randreal01() = delete;

template<>
[[nodiscard]] inline f32 randreal01()
{
    std::uniform_real_distribution<f32> distribution(0.0f, 1.0f);
    return distribution(gen);
}

template<>
[[nodiscard]] inline f64 randreal01()
{
    std::uniform_real_distribution<f64> distribution(0.0, 1.0);
    return distribution(gen);
}


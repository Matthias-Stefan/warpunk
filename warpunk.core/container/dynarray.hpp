#pragma once

#include "warpunk.core/defines.h"
#include "warpunk.core/platform/platform.h"

template<typename T>
struct dynarray_t
{
    s64 size;
    s64 capacity;
    T* data;
};

template<typename T>
warpunk_api inline dynarray_t<T> dynarray_empty()
{
    dynarray_t<T> array = {};
    array.size = 0;
    array.capacity = 0;
    array.data = nullptr;

    return array;
}

template<typename T>
warpunk_api inline dynarray_t<T> dynarray_create(u64 size)
{
    if (size == 0)
    {
        return dynarray_empty<T>();
    }

    dynarray_t<T> array = {};
    array.data = (T *)platform_memory_alloc(sizeof(T) * size); 
    platform_memory_zero(array.data, sizeof(T) * size);
    array.size = size;
    array.capacity = size;
    return array;
}

template<typename T>
warpunk_api inline void dynarray_destroy(dynarray_t<T>* array)
{
    platform_memory_free(array->data);
    array->data = nullptr;
    array->size = 0;
    array->capacity = 0;
}

template<typename T>
warpunk_api inline T dynarray_front(dynarray_t<T>* array)
{
    if (array->size == 0)
    {
        return T {};
    }

    return array->data[0];
}

template<typename T>
warpunk_api inline T dynarray_back(dynarray_t<T>* array)
{
    if (array->size == 0)
    {
        return T {};
    }
    
    return array->data[array->capacity-1];
}

template<typename T>
warpunk_api inline b8 dynarray_resize(dynarray_t<T>* array, s64 size)
{
    if (array->data == nullptr)
    {
        return false;
    }

    void* temp = platform_memory_alloc(sizeof(T) * size);
    if (temp == nullptr)
    {
        return false;
    } 
    platform_memory_zero(temp, sizeof(T) * size);
    platform_memory_copy(temp, array->data, sizeof(T) * size);
    platform_memory_free(array->data);
    array->data = (T *)temp;
    array->size = size;

    return true;
}

template<typename T>
warpunk_api inline b8 dynarray_shrink_to_fit(dynarray_t<T>* array)
{
    return dynarray_resize(array, array->capacity);
}

template<typename T>
warpunk_api inline void dynarray_add(dynarray_t<T>* array, T element)
{
    if (array->size == 0)
    {
        array->data = (T *)platform_memory_alloc(sizeof(T) * 1); 
        platform_memory_zero(array->data, sizeof(T) * 1);
        array->size = 1;
        array->capacity = 0;
    }
    else if (array->capacity == array->size)
    {
        if (!dynarray_resize(array, array->size * 2))
        {
            // TODO: error
            return;
        }
    }

    array->data[array->capacity++] = element;
}

template<typename T>
warpunk_api inline void dynarray_remove(dynarray_t<T>* array)
{
    if (array->capacity == 0)
    {
        return;
    }
    array->data[array->capacity--] = T {};
}

template<typename T>
warpunk_api inline void dynarray_remove_at(dynarray_t<T>* array, s64 index)
{
    if (index >= array->size)
    {
        // TODO: error
        return;
    }

    void* dst = ((T *)array->data) + index;
    void* src = ((T *)array->data) + index+1;
    platform_memory_copy(dst, src, sizeof(T) * (array->capacity - index));

    array->data[array->capacity--] = T {};
}

template<typename T>
warpunk_api inline void dynarray_clear(dynarray_t<T>* array)
{
    platform_memory_zero(array->data, sizeof(T) * array->size);
    array->capacity = 0;
}

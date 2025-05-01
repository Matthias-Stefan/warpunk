#pragma once

#include "warpunk.core/defines.h"
#include "warpunk.core/platform/platform.h"

/** */
template<typename T>
struct stcqueue_s
{
    s64 size;
    s64 capacity;
    T* data;
    s64 head;
    s64 tail;
};

/** */
template<typename T>
warpunk_api inline stcqueue_s<T> stcqueue_create(s64 size)
{
    if (size <= 0)
    {
        size = 1;
    }

    stcqueue_s<T> queue = {};
    queue.size = size;
    queue.capacity = 0;
    queue.data = (T *)platform_memory_alloc(sizeof(T) * size);
    platform_memory_zero(queue.data, sizeof(T) * size);
    queue.head = 0;
    queue.tail = 0;
    return queue;
}

/** */
template<typename T>
warpunk_api inline void stcqueue_destroy(stcqueue_s<T>* queue)
{   
    platform_memory_free(queue->data);
    queue->size = 0;
    queue->capacity = 0;
    queue->data = nullptr;
    queue->head = 0;
    queue->tail = 0;
}

/** */
template<typename T>
warpunk_api inline b8 stcqueue_enqueue(stcqueue_s<T>* queue, T element)
{
    if (queue->capacity == queue->size)
    {
        return false;
    }

    queue->data[queue->tail] = element;
    queue->capacity++;
    queue->tail = (queue->tail + 1) & queue->size;
    return true;
}

/** */
template<typename T>
warpunk_api inline T stcqueue_dequeue(stcqueue_s<T>* queue)
{
    if (queue->head == queue->tail)
    {
        return T {};
    }
    
    T element = queue->data[queue->head];
    queue->data[queue->head] = T {};
    queue->head = (queue->head + 1) % queue->size;
    queue->capacity--;
    return element;
}

/** */
template<typename T>
warpunk_api inline void stcqueue_clear(stcqueue_s<T>* queue)
{
    platform_memory_zero(queue->data, sizeof(T) * queue->size);
    queue->capacity = 0;
    queue->head = 0;
    queue->tail = 0;
}


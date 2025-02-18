#pragma once

#include "warpunk.core/defines.h"
#include "warpunk.core/platform/platform.h"

/** */
template<typename T>
struct dynqueue_t
{
    s64 size;
    s64 capacity;
    T* data;
    s64 head;
    s64 tail;
};

/** */
template<typename T>
warpunk_api inline dynqueue_t<T> dynqueue_create(s64 size)
{
    if (size <= 0)
    {
        size = 1;
    }

    dynqueue_t<T> queue = {};
    queue.data = (T *)platform_memory_alloc(sizeof(T) * size);
    platform_memory_zero(queue.data, sizeof(T) * size);
    queue.head = 0;
    queue.tail = 0;
    queue.size = size;
    queue.capacity = 0;
    return queue;
}

/** */
template<typename T>
warpunk_api inline void dynqueue_destroy(dynqueue_t<T>* queue)
{
    platform_memory_free(queue->data);
    queue->data = nullptr;
    queue->head = 0;
    queue->tail = 0;
    queue->size = 0;
    queue->capacity = 0;
}

/** */
template<typename T>
warpunk_api inline b8 dynqueue_enqueue(dynqueue_t<T>* queue, T element)
{
    queue->data[queue->tail] = element;
    queue->capacity++;

    s64 next_element = (queue->tail + 1) % queue->size;
    if (next_element == queue->head)
    {
        void* temp = platform_memory_alloc(sizeof(T) * queue->size * 2); 
        T* temp_typed = (T *)temp;
        if (temp == nullptr)
        {
            return false;
        }
        platform_memory_zero(temp, sizeof(T) * queue->size * 2);

        /** copy to boundary */
        platform_memory_copy(temp_typed, &((T *)queue->data)[queue->head], sizeof(T) * (queue->size - queue->head));
        temp_typed += queue->size - queue->head;
        /** copy rest */
        platform_memory_copy(temp_typed, queue->data, sizeof(T) * (queue->tail + 1));
        platform_memory_free(queue->data);
        queue->head = 0;
        queue->size *= 2;
        queue->tail = queue->capacity;
        queue->data = (T *)temp;
    }
    else 
    {
        queue->tail = next_element;
    }

    return true;
}

/** */
template<typename T>
warpunk_api inline T dynqueue_dequeue(dynqueue_t<T>* queue)
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
warpunk_api inline void dynqueue_clear(dynqueue_t<T>* queue)
{
    platform_memory_zero(queue->data, sizeof(T) * queue->size);
    queue->capacity = 0;
    queue->head = 0;
    queue->tail = 0;
}

#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define vulkan_eval_result(vk_result)                            \
    do {                                                         \
        VkResult error = vk_result;                              \
        if (error)                                               \
        {                                                        \
            fprintf(stderr, "Vulkan error: %d at %s:%d\n",       \
                    error, __FILE__, __LINE__);                  \
            abort();                                             \
        }                                                        \
    } while (0)




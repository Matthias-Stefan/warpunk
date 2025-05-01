#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

#include "warpunk.core/defines.h"
#include "warpunk.core/container/dynarray.hpp"
#include "warpunk.core/renderer/renderer_types.h"

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

namespace vulkan_renderer
{
    typedef struct _vulkan_physical_device_requirements_s
    {
        b8 graphics;
        b8 present;
        b8 compute;
        b8 transfer;
    
        dynarray_s<const char*> device_extension_names;
        b8 sampler_anisotropy;
        b8 discrete_gpu;
    } vulkan_physical_device_requirements_s;

    typedef struct _vulkan_queue_family_indices_s
    { 
        s32 graphics_queue_index;
        s32 present_queue_index;
        s32 compute_queue_index;
        s32 transfer_queue_index;
    } vulkan_queue_family_indices_s;

    typedef struct _vulkan_swapchain_support_info_s
    {
        VkSurfaceCapabilitiesKHR capabilities;
        u32 format_count;
        VkSurfaceFormatKHR* formats;
        u32 present_mode_count;
        VkPresentModeKHR* present_modes;
    } vulkan_swapchain_support_info_s;


    typedef struct _vulkan_state_s
    {
        b8 is_initialized;
        VkInstance instance;
        // VkAllocationCallbacks allocator;
        VkSurfaceKHR surface;
        renderer_config_flag config_flags;

        // NOTE: physical device
        VkPhysicalDevice physical_device = VK_NULL_HANDLE; 
       
        // NOTE: queues
        vulkan_queue_family_indices_s queue_family_indices;
        VkQueue graphics_queue;
        VkQueue present_queue;
        VkQueue compute_queue;
        VkQueue transfer_queue;
   
        // NOTE: info 
        u32 api_major;
        u32 api_minor;
        u32 api_patch;
        VkPhysicalDeviceProperties physical_device_properties;
        VkPhysicalDeviceFeatures physical_device_features;
        VkPhysicalDeviceMemoryProperties physical_device_memory_properties;
        b8 supports_dynamic_state_natively;
        b8 supports_dynamic_state;
        b8 supports_smooth_lines;
    
        // NOTE: device
        VkDevice device;
        VkCommandPool command_pool;
 
        // NOTE: swapchain
        vulkan_swapchain_support_info_s swapchain_support;
    } vulkan_state_s;
    
    typedef struct _vulkan_swapchain_s
    {
        VkSurfaceFormatKHR image_format;
        VkSwapchainKHR handle;
        u32 image_count;
        b8 supports_blit_dst;
        b8 supports_blit_src;
        u32* color_texture;
        u32 image_index;
    } vulkan_swapchain_s;
}

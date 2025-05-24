#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

#include "warpunk.core/src/defines.h"
#include "warpunk.core/src/container/dynarray.hpp"
#include "warpunk.core/src/renderer/renderer_types.h"

#define vulkan_eval_result(vk_result)               \
    do {                                            \
        VkResult error = vk_result;                 \
        if (error)                                  \
        {                                           \
            WERROR("Vulkan error: %d at %s:%d\n",   \
                    error, __FILE__, __LINE__);     \
            abort();                                \
        }                                           \
    } while (0)

namespace vulkan_renderer
{
    struct vulkan_context;

    typedef struct vulkan_swapchain_support_info
    {
        /** @brief Surface capabilities such as min/max image count, extent, etc. */
        VkSurfaceCapabilitiesKHR capabilities;
        /** @brief Number of supported surface formats. */
        u32 format_count;
        /** @brief Array of supported surface formats. */
        VkSurfaceFormatKHR* formats;
        /** @brief Number of supported present modes. */
        u32 present_mode_count;
        /** @brief Array of supported present modes. */
        VkPresentModeKHR* present_modes;
    } vulkan_swapchain_support_info;


    typedef struct vulkan_device
    {
        /** @brief Vulkan API major version supported. */
        u32 api_major;
        /** @brief Vulkan API minor version supported. */
        u32 api_minor;
        /** @brief Vulkan API patch version supported. */
        u32 api_patch;

        /** @brief The selected physical device (GPU). */
        VkPhysicalDevice physical_device;
        /** @brief The created logical device. */
        VkDevice logical_device;
        /** @brief Swapchain support details for the device. */
        vulkan_swapchain_support_info swapchain_support;
        
        /** @brief Queue index for graphics operations. */
        s32 graphics_queue_index;
        /** @brief Queue index for presentation. */
        s32 present_queue_index;
        /** @brief Queue index for computation. */
        s32 compute_queue_index;
        /** @brief Queue index for transfer operations. */
        s32 transfer_queue_index;
        /** @brief Indicates if the device supports host-visible local memory. */
        b8 supports_device_local_host_visible;

        /** @brief Handle to the graphics queue. */
        VkQueue graphics_queue;
        /** @brief Handle to the presentation queue. */
        VkQueue present_queue;
        /** @brief Handle to the compute queue. */
        VkQueue compute_queue;
        /** @brief Handle to the transfer queue. */
        VkQueue transfer_queue;

        /** @brief Command pool for allocating graphics command buffers. */
        VkCommandPool graphics_command_pool;

        /** @brief Properties of the selected physical device. */
        VkPhysicalDeviceProperties physical_device_properties;
        /** @brief Supported features of the selected physical device. */
        VkPhysicalDeviceFeatures physical_device_features;
        /** @brief Memory properties of the selected physical device. */
        VkPhysicalDeviceMemoryProperties physical_device_memory_properties;

        /** @brief Chosen depth format for depth buffering. */
        VkFormat depth_format;
        /** @brief Number of channels in the selected depth format. */
        u8 depth_channel_count;

        /** @brief Indicates if dynamic state is supported natively by the GPU. */
        b8 supports_dynamic_state_natively;
        /** @brief Indicates if dynamic state is supported (natively or via fallback). */
        b8 supports_dynamic_state;
        /** @brief Indicates if smooth (antialiased) lines are supported. */
        b8 supports_smooth_lines;
    } vulkan_device;

    typedef struct vulkan_swapchain
    {
        /** @brief Format of the images used in the swapchain (e.g., color space and pixel format). */
        VkSurfaceFormatKHR image_format;
        /** @brief Handle to the Vulkan swapchain. */
        VkSwapchainKHR handle;
        /** @brief Number of images in the swapchain. */
        u32 image_count;
        /** @brief Indicates if the swapchain supports blitting as a destination. */
        b8 supports_blit_dst;
        /** @brief Indicates if the swapchain supports blitting as a source. */
        b8 supports_blit_src;
        /** @brief Array of handles to color textures (render targets). */
        u32* color_texture;
        /** @brief Index of the currently acquired swapchain image. */
        u32 image_index;
    } vulkan_swapchain;

    typedef struct warpunk_window_renderer_backend_state
    {
        VkSurfaceKHR surface;
        vulkan_swapchain swapchain;

    } warpunk_window_renderer_backend_state;

    typedef struct vulkan_context
    {
        b8 is_initialized;
        u32 api_major;
        u32 api_minor;
        u32 api_patch;

        VkInstance instance;
        VkAllocationCallbacks* allocator;
        VkSurfaceKHR surface;
        renderer_config_flag config_flags;
       
        VkPhysicalDeviceProperties physical_device_properties;
        VkPhysicalDeviceFeatures physical_device_features;
        VkPhysicalDeviceMemoryProperties physical_device_memory_properties;
        b8 supports_dynamic_state_natively;
        b8 supports_dynamic_state;
        b8 supports_smooth_lines;
    
        vulkan_device device;
        VkCommandPool graphics_command_pool;
    } vulkan_context;
    


}

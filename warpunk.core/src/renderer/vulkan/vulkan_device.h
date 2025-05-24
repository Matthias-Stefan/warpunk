#pragma once

#include "warpunk.core/src/defines.h"
#include "warpunk.core/src/renderer/vulkan/vulkan_types.h"

namespace vulkan_renderer 
{
    /**
    * @brief Creates and initializes a Vulkan device for the given context.
    */
    b8 vulkan_device_create(vulkan_context* context);

    /**
    * @brief Destroys and cleans up the Vulkan device associated with the given context.
    */
    void vulkan_device_destroy(vulkan_context* context);

    /**
    * @brief Queries the swapchain support details for a physical device and surface.
    *
    * @param context The Vulkan context.
    * @param physical_device The physical device to query.
    * @param surface The Vulkan surface to evaluate.
    * @param out_support_info Output structure to receive the swapchain support information.
    */
    void vulkan_device_query_swapchain_support(
        vulkan_context* context, 
        VkPhysicalDevice physical_device,
        VkSurfaceKHR surface, 
        vulkan_swapchain_support_info* out_support_info);

    /**
    * @brief Attempts to find and set a supported depth format for the given device.
    *
    * @param context The Vulkan context.
    * @param device The Vulkan device.
    * @return True if a suitable depth format was found, false otherwise.
    */
    b8 vulkan_device_detect_depth_format(vulkan_context* context, vulkan_device* device);
}
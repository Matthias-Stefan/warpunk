#pragma once

#include "warpunk.core/src/defines.h"

namespace vulkan_renderer 
{
    b8 vulkan_swapchain_create(
            const struct vulkan_state* const state, 
            struct vulkan_swapchainupport_info* swapchain_support,
            u32 width, u32 height,
            struct vulkan_swapchain* out_swapchain);

    b8 vulkan_swapchain_recreate();

    b8 vulkan_swapchain_destroy();
}

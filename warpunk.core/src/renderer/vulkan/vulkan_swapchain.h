#pragma once

#include "warpunk.core/src/defines.h"
#include "warpunk.core/src/renderer/vulkan/vulkan_types.h"

namespace vulkan_renderer 
{
    b8 vulkan_swapchain_create(
            vulkan_context* context, 
            u32 width, 
            u32 height,
            vulkan_swapchain* out_swapchain);

    b8 vulkan_swapchain_recreate();

    b8 vulkan_swapchain_destroy();
}

#pragma once

#include "warpunk.core/defines.h"

namespace vulkan_renderer 
{
    b8 vulkan_swapchain_create(
            const struct _vulkan_state_s* const state, 
            struct _vulkan_swapchain_support_info_s* swapchain_support,
            u32 width, u32 height,
            struct _vulkan_swapchain_s* out_swapchain);

    b8 vulkan_swapchain_recreate();

    b8 vulkan_swapchain_destroy();
}

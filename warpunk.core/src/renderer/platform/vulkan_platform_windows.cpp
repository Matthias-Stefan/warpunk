#include "warpunk.core/src/defines.h"
#if defined(WARPUNK_WINDOWS)

#include <vulkan/vulkan_core.h>

namespace vulkan_renderer
{
    b8 vulkan_platform_create_surface(VkInstance instance, VkAllocationCallbacks* allocator, VkSurfaceKHR* surface)
    {
        return true;
    }

    b8 vulkan_platform_get_required_instance_extensions(u32* extension_count, const char*** extensions)
    {
        return true;
    }

    b8 vulkan_platform_presentation_support(VkPhysicalDevice physical_device, u32 queue_family_index)
    {
        return true;
    }
}

#endif
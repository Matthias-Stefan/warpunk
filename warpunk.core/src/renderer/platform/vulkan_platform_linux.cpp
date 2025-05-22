#include "warpunk.core/src/defines.h"
#if defined(WARPUNK_LINUX)
#include "warpunk.core/src/renderer/platform/vulkan_platform.h"

#include <xcb/xcb.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_xcb.h>

#include "warpunk.core/src/platform/platform.h"
#include "warpunk.core/src/platform/platform_linux.h"
#include "warpunk.core/src/renderer/vulkan/vulkan_types.h"

namespace vulkan_renderer
{
    b8 vulkan_platform_create_surface(VkInstance instance, VkAllocationCallbacks* allocator, VkSurfaceKHR* surface)
    {
        if (instance == VK_NULL_HANDLE)
        {
            return false;
        }

        linux_handle_s platform_handle;
        if (!platform_get_linux_handle(&platform_handle))
        {
            return false;
        }

        VkXcbSurfaceCreateInfoKHR xcb_surface_create_info = {};
        xcb_surface_create_info.sType = VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR;
        xcb_surface_create_info.pNext = NULL;
        xcb_surface_create_info.flags = 0;
        xcb_surface_create_info.window = platform_handle.window;
        xcb_surface_create_info.connection = platform_handle.connection;

        vulkan_eval_result(vkCreateXcbSurfaceKHR(instance, &xcb_surface_create_info, nullptr, surface));

        return true;
    }

    b8 vulkan_platform_get_required_instance_extensions(u32 *extension_count, const char ***extensions)
    {
        const char* instance_extensions[] = {
            "VK_KHR_surface",
            "VK_KHR_xcb_surface"
        };

        *extension_count = 2;
        if (extensions != nullptr)
        {
            platform_memory_copy(*extensions, instance_extensions, sizeof(instance_extensions)); 
        }

        return true; 
    }

    b8 vulkan_platform_presentation_support(VkPhysicalDevice physical_device, u32 queue_family_index)
    {
        linux_handle_s platform_handle;
        if (!platform_get_linux_handle(&platform_handle))
        {
            return false;
        }

        return (b8)vkGetPhysicalDeviceXcbPresentationSupportKHR(physical_device, 
                queue_family_index, platform_handle.connection, platform_handle.screen->root_visual);  
    }
}

#endif // WARPUNK_LINUX 

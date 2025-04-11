#include "warpunk.core/defines.h"
#if defined(WARPUNK_LINUX)
#include "warpunk.core/renderer/platform/vulkan_platform.h"

#include <xcb/xcb.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_xcb.h>

#include "warpunk.core/platform/platform_linux.h"
#include "warpunk.core/renderer/renderer_backend_vulkan_types.h"

namespace vulkan_renderer
{
    b8 vulkan_platform_create_surface(VkInstance instance, VkAllocationCallbacks* allocator, VkSurfaceKHR* surface)
    {
        if (instance == VK_NULL_HANDLE)
        {
            return false;
        }

        xcb_connection_t* connection = nullptr;
        xcb_window_t window = 0;

        if (!platform_linux_get_connection(&connection) || !platform_linux_get_window(&window))
        {
            return false;
        }

        VkXcbSurfaceCreateInfoKHR xcb_surface_create_info = {};
        xcb_surface_create_info.sType = VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR;
        xcb_surface_create_info.pNext = NULL;
        xcb_surface_create_info.flags = 0;
        xcb_surface_create_info.window = window;y
        xcb_surface_create_info.connection = connection;

        vulkan_eval_result(vkCreateXcbSurfaceKHR(instance, &xcb_surface_create_info, nullptr, surface));

        return true;
    }
}

#endif // WARPUNK_LINUX 

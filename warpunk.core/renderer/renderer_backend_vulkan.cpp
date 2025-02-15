#include "warpunk.core/renderer/renderer_backend.h"

#include <vulkan.h>

typedef struct  _vulkan_state_t
{
    b8 is_initialized;
    VkInstance instance;
    VkAllocationCallbacks allocation_callbacks;
} vulkan_state_t;

static vulkan_state_t state;

// TODO: in renderer_config
// - application name
// - engine name
static const char* application_name = "mm";
static const char* engine_name = "warpunk";

namespace vulkan_renderer
{
    [[nodiscard]] b8 vulkan_eval_result(VkResult vk_result)
    {
        return vk_result == VK_SUCCESS;
    }

    [[nodiscard]] b8 renderer_startup(renderer_config_t renderer_config)
    {
        if (state.is_initialized)
        {
            return true;
        }
        
        VkApplicationInfo application_info = {};
        application_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        application_info.pApplicationName = application_name;
        application_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        application_info.pEngineName = engine_name;
        application_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        application_info.apiVersion = VK_API_VERSION_1_4;

        VkInstanceCreateInfo instance_create_info = {};
        instance_create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        instance_create_info.pApplicationInfo = &application_info;
        instance_create_info.enabledLayerCount = 0;
        instance_create_info.ppEnabledLayerNames = nullptr;
        instance_create_info.enabledExtensionCount = 0;
        instance_create_info.ppEnabledExtensionNames = nullptr;

        if (!vulkan_eval_result(vkCreateInstance(&instance_create_info, nullptr, &state.instance)))
        { 
            return false;
        }

        return true;
    }
}

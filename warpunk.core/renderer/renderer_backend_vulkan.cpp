#include "warpunk.core/renderer/renderer_backend.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <vulkan/vulkan.h>

#include "warpunk.core/container/dynarray.hpp"


namespace vulkan_renderer 
{
    static b8 check_validation_layer_support(dynarray_t<const char*>* validation_layers);
#if true
    static b8 is_device_suitable(VkPhysicalDevice physical_device);
    // TODO: instead of is device suitable implement rate 
    // static s32 rate_device_suitability(VkPhysicalDevice physical_device);
#endif
    static u32 find_queue_families(VkPhysicalDevice physical_device);
}

typedef struct  _vulkan_state_t
{
    b8 is_initialized;
    VkInstance instance;
    VkAllocationCallbacks allocation_callbacks;

    VkPhysicalDevice physical_device = VK_NULL_HANDLE; 
} vulkan_state_t;

typedef struct _queue_family_indices_t
{

} queue_family_indices_t;

static vulkan_state_t state;

// TODO: in renderer_config
// - application name
// - engine name
static const char* application_name = "mm";
static const char* engine_name = "warpunk";

namespace vulkan_renderer
{
    #define vulkan_eval_result(vk_result)                        \
    do {                                                         \
        VkResult error = vk_result;                              \
        if (error)                                               \
        {                                                        \
            fprintf(stderr, "Vulkan error: %d at %s:%d\n",       \
                    error, __FILE__, __LINE__);                  \
            abort();                                             \
        }                                                        \
    } while (0)


    b8 renderer_startup(renderer_config_t renderer_config)
    {
        if (state.is_initialized)
        {
            return true;
        }
       
        /** begin instance */

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

        /** begin validation layers */

        dynarray_t<const char*> validation_layers = dynarray_create<const char *>(1);
        dynarray_add(&validation_layers, "VK_LAYER_KHRONOS_validation");
#ifdef WARPUNK_DEBUG
        const b8 enable_validation_layers = true;
#else
        const b8 enable_validation_layers = false;    
#endif
        if (enable_validation_layers && !check_validation_layer_support(&validation_layers))
        {
            return false;
        }

        if (enable_validation_layers)
        {
            instance_create_info.enabledLayerCount = validation_layers.capacity;
            instance_create_info.ppEnabledLayerNames = validation_layers.data;
        }

        vulkan_eval_result(vkCreateInstance(&instance_create_info, nullptr, &state.instance));
        dynarray_destroy(&validation_layers);
        
        /** begin pick physical device */

        u32 device_count = 0;
        vkEnumeratePhysicalDevices(state.instance, &device_count, nullptr);
        if (device_count == 0)
        {
            fprintf(stderr, "No Vulkan-compatible physical devices found.\n");
            return false;
        }
        dynarray_t<VkPhysicalDevice> physical_devices = dynarray_create<VkPhysicalDevice>(device_count);
        vkEnumeratePhysicalDevices(state.instance, &device_count, physical_devices.data);
        
        for (s32 device_index = 0; device_index < device_count; ++device_index)
        {
            if (is_device_suitable(physical_devices.data[device_index]))
            {
                state.physical_device = physical_devices.data[device_index];
                break;
            }
        }
        
        dynarray_destroy(&physical_devices);
        if (state.physical_device == VK_NULL_HANDLE)
        {
            fprintf(stderr, "No suitable Vulkan physical device found.\n");
            return false;
        }




        
        return true;
    }


    b8 renderer_shutdown()
    {
        vkDestroyInstance(state.instance, &state.allocation_callbacks);

        return true;
    }


    static b8 check_validation_layer_support(dynarray_t<const char*>* validation_layers)
    {
        u32 layer_count;
        vkEnumerateInstanceLayerProperties(&layer_count, nullptr);
        dynarray_t<VkLayerProperties> available_layers = dynarray_create<VkLayerProperties>(layer_count);
        vkEnumerateInstanceLayerProperties(&layer_count, available_layers.data);
        
        for (u32 layer_idx = 0; layer_idx < validation_layers->capacity; ++layer_idx)
        {
            b8 layer_found = false;
            const char* layer_name = validation_layers->data[layer_idx];
            
            for (u32 available_layer_idx = 0; available_layer_idx < layer_count; ++available_layer_idx)
            {
                const char* available_layer_name = available_layers.data[available_layer_idx].layerName;
                if (strcmp(layer_name, available_layer_name) == 0)
                {
                    layer_found = true;
                    break;
                }
            }

            if (!layer_found)
            {
                dynarray_destroy(&available_layers);
                return false;
            }

        }

        dynarray_destroy(&available_layers);
        return true;
    }


    static b8 is_device_suitable(VkPhysicalDevice physical_device)
    {
        VkPhysicalDeviceProperties physical_device_properties = {};
        VkPhysicalDeviceFeatures physical_device_features = {};
        vkGetPhysicalDeviceProperties(physical_device, &physical_device_properties);
        vkGetPhysicalDeviceFeatures(physical_device, &physical_device_features);

        return physical_device_properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU && physical_device_features.geometryShader;
    }

    
    static u32 find_queue_families(VkPhysicalDevice physical_device)
    {
        
    }
}

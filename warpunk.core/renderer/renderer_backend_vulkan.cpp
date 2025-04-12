#include "warpunk.core/renderer/renderer_backend.h"

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

#include "warpunk.core/container/dynarray.hpp"
#include "warpunk.core/renderer/platform/vulkan_platform.h"
#include "warpunk.core/renderer/renderer_backend_vulkan_types.h"

typedef struct  _vulkan_state_t
{
    b8 is_initialized;
    VkInstance instance;
    VkAllocationCallbacks allocator;
    VkSurfaceKHR surface;

    VkPhysicalDevice physical_device = VK_NULL_HANDLE; 
} vulkan_state_t;

typedef struct _queue_family_indices_s
{ 
    s32 graphics_family_index;
    s32 present_family_index;
    s32 compute_family_index;
    s32 transfer_family_index;
} queue_family_indices_s;


namespace vulkan_renderer 
{
    static b8 check_validation_layer_support(dynarray_t<const char*>* validation_layers);
#if true
    static b8 is_device_suitable(VkPhysicalDevice physical_device);
    // TODO: instead of is device suitable implement rate 
    // static s32 rate_device_suitability(VkPhysicalDevice physical_device);
#endif
    static queue_family_indices_s find_queue_families(VkPhysicalDevice physical_device);
}


static vulkan_state_t state;

// TODO: in renderer_config
// - application name
// - engine name
static const char* application_name = "mm";
static const char* engine_name = "warpunk";


namespace vulkan_renderer
{
    b8 renderer_startup(renderer_config_t renderer_config)
    {
        if (state.is_initialized)
        {
            return true;
        }
       
        /** begin instance */
        u32 instance_extension_count = 0;
        vulkan_platform_get_required_instance_extensions(&instance_extension_count, nullptr);
        dynarray_t<const char *> instance_extensions = dynarray_create<const char *>(instance_extension_count);
        vulkan_platform_get_required_instance_extensions(&instance_extension_count, &instance_extensions.data);

#if WARPUNK_DEBUG
        instance_extensions.capacity = instance_extension_count; 
        instance_extension_count += 1;
        dynarray_add(&instance_extensions, "VK_EXT_debug_utils");
#endif
        
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
        instance_create_info.enabledExtensionCount = instance_extension_count;
        instance_create_info.ppEnabledExtensionNames = instance_extensions.data;

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
        dynarray_destroy(&instance_extensions);
        dynarray_destroy(&validation_layers);

        /** begin surface */

        if (!vulkan_platform_create_surface(state.instance, &state.allocator, &state.surface))
        {
            fprintf(stderr, "Failed to create surface");
            return false;
        }
        
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
        vkDestroyInstance(state.instance, &state.allocator);

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

       if (physical_device_properties.deviceType != VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
       {
           return false;
       }

        queue_family_indices_s queue_family_indices = find_queue_families(physical_device);

        return physical_device_properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU && physical_device_features.geometryShader;
    }

    
    static queue_family_indices_s find_queue_families(VkPhysicalDevice physical_device)
    {
        queue_family_indices_s indices = {};
        indices.graphics_family_index = -1;
        indices.present_family_index = -1;
        indices.compute_family_index = -1;
        indices.transfer_family_index = -1;

        u32 queue_family_properties_count = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_properties_count, nullptr);
        if (queue_family_properties_count > 0)
        {
            dynarray_t<VkQueueFamilyProperties> queue_families = dynarray_create<VkQueueFamilyProperties>(queue_family_properties_count);
            vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_properties_count, queue_families.data);
            for (int family_index = 0; family_index < queue_family_properties_count; ++family_index)
            {
                if (queue_families.data[family_index].queueFlags & VK_QUEUE_GRAPHICS_BIT)
                {
                    //vkGetPhysicalDeviceSurfaceSupportKHR(physical_device, family_index, VkSurfaceKHR surface, VkBool32 *pSupported)
                    indices.graphics_family_index = family_index;
                }
            }

                dynarray_destroy(&queue_families);
        }


        return indices;
    }
}

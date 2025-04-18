#include "warpunk.core/renderer/renderer_backend.h"

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

#include "warpunk.core/container/dynarray.hpp"
#include "warpunk.core/renderer/platform/vulkan_platform.h"
#include "warpunk.core/renderer/renderer_backend_vulkan_types.h"

typedef struct _vulkan_physical_device_requirements_s
{
    b8 graphics;
    b8 present;
    b8 compute;
    b8 transfer;
    
    dynarray_s<const char*> device_extension_names;
    b8 sampler_anisotropy;
    b8 discrete_gpu;
} vulkan_physical_device_requirements_s;

typedef struct _vulkan_queue_family_indices_s
{ 
    s32 graphics_queue_index;
    s32 present_queue_index;
    s32 compute_queue_index;
    s32 transfer_queue_index;
} vulkan_queue_family_indices_s;

typedef struct _vulkan_state_s
{
    b8 is_initialized;
    VkInstance instance;
    VkAllocationCallbacks allocator;
    VkSurfaceKHR surface;

    /** NOTE: physical device */
    VkPhysicalDevice physical_device = VK_NULL_HANDLE; 
    vulkan_queue_family_indices_s queue_family_indices;
    u32 api_major;
    u32 api_minor;
    u32 api_patch;
    VkPhysicalDeviceProperties physical_device_properties;
    VkPhysicalDeviceFeatures physical_device_features;
    VkPhysicalDeviceMemoryProperties physical_device_memory_properties;
    b8 supports_dynamic_state_natively;
    b8 supports_dynamic_state;
    b8 supports_smooth_lines;


    VkDevice device;
} vulkan_state_s;


namespace vulkan_renderer 
{
    static b8 check_validation_layer_support(dynarray_s<const char*>* validation_layers);


    static b8 select_physical_device(const vulkan_physical_device_requirements_s* requirements);
    static b8 physical_device_is_suitable(VkPhysicalDevice physical_device,
                                          const VkPhysicalDeviceProperties* properties,
                                          const VkPhysicalDeviceFeatures* features,
                                          const vulkan_physical_device_requirements_s* requirements,
                                          vulkan_queue_family_indices_s* out_queue_family_indices);
}

static vulkan_state_s state;

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
       
        /** NOTE: instance */
        u32 instance_extension_count = 0;
        vulkan_platform_get_required_instance_extensions(&instance_extension_count, nullptr);
        dynarray_s<const char *> instance_extensions = dynarray_create<const char *>(instance_extension_count);
        vulkan_platform_get_required_instance_extensions(&instance_extension_count, &instance_extensions.data);

#if WARPUNK_DEBUG
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

        /** NOTE: validation layers */

        dynarray_s<const char*> validation_layers = dynarray_empty<const char *>();
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

        /** NOTE: surface */

        if (!vulkan_platform_create_surface(state.instance, &state.allocator, &state.surface))
        {
            fprintf(stderr, "Failed to create surface");
            return false;
        }
        
        /** NOTE: physical device */

        vulkan_physical_device_requirements_s requirements = {};
        requirements.graphics = true;
        requirements.present = true;
        requirements.compute = true;
        requirements.transfer = true;
        requirements.device_extension_names = dynarray_empty<const char*>();
        dynarray_add(&requirements.device_extension_names, "VK_KHR_swapchain");
        requirements.sampler_anisotropy = true;
        requirements.discrete_gpu = true;

        if (!select_physical_device(&requirements))
        {
            dynarray_destroy(&requirements.device_extension_names);
            return false;
        }

        dynarray_destroy(&requirements.device_extension_names);
        
        /** NOTE: logical device */
    
        /** NOTE: queues */
        b8 present_shares_graphics_queue = state.queue_family_indices.graphics_queue_index == state.queue_family_indices.present_queue_index;
        b8 transfer_shares_graphics_queue = state.queue_family_indices.graphics_queue_index == state.queue_family_indices.transfer_queue_index;
        b8 present_must_share_graphics = false;
        u32 index_count = 1;
        if (!present_shares_graphics_queue)
        {
            index_count++;
        }
        if (!transfer_shares_graphics_queue)
        {
            index_count++;
        }
        s32 queue_indices[32];
        u8 queue_index = 0;
        queue_indices[queue_index++] = state.queue_family_indices.graphics_queue_index;
        if (!present_shares_graphics_queue)
        {
            queue_indices[queue_index++] = state.queue_family_indices.present_queue_index;
        }
        if (!transfer_shares_graphics_queue)
        {
            queue_indices[queue_index++] = state.queue_family_indices.transfer_queue_index;
        }

        dynarray_s<VkDeviceQueueCreateInfo> queue_create_info = dynarray_create<VkDeviceQueueCreateInfo>(index_count);        
        f32 queue_priorities[2] = { 0.9f, 1.0f };
       
        // TODO: dynamic
        VkQueueFamilyProperties props[64];
        u32 prop_count;
        vkGetPhysicalDeviceQueueFamilyProperties(state.physical_device, &prop_count, nullptr);
        vkGetPhysicalDeviceQueueFamilyProperties(state.physical_device, &prop_count, props);

        for (u32 i = 0; i < index_count; ++i) {
            queue_create_info.data[i].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queue_create_info.data[i].queueFamilyIndex = queue_indices[i];
            queue_create_info.data[i].queueCount = 1;
    
            if (present_shares_graphics_queue && queue_indices[i] == state.queue_family_indices.present_queue_index) 
            {
                if (props[state.queue_family_indices.present_queue_index].queueCount > 1) 
                {
                    queue_create_info.data[i].queueCount = 2;
                } 
                else 
                {
                    present_must_share_graphics = true;
                }
            }
    
            if (queue_indices[i] == state.queue_family_indices.graphics_queue_index) 
            {
                 queue_create_info.data[i].queueCount = 2;
            }
            queue_create_info.data[i].flags = 0;
            queue_create_info.data[i].pNext = 0;
            queue_create_info.data[i].pQueuePriorities = queue_priorities;
        }  

        /** NOTE: device extension */
        u32 available_device_extension_count = 0;
        vulkan_eval_result(vkEnumerateDeviceExtensionProperties(state.physical_device, nullptr, &available_device_extension_count, nullptr));
        dynarray_s<VkExtensionProperties> device_extension_properties = dynarray_empty<VkExtensionProperties>();
        if (available_device_extension_count > 0)
        {
            device_extension_properties = dynarray_create<VkExtensionProperties>(available_device_extension_count); 
            vulkan_eval_result(vkEnumerateDeviceExtensionProperties(state.physical_device, nullptr, &available_device_extension_count, device_extension_properties.data));
        }

        // TODO:
        dynarray_s<const char*> device_extension_names;

        // TODO:
        //vkCreateDevice(state.physical_device, &device_create_info, &state.allocator, &state.device); 
        dynarray_destroy(&queue_create_info);
        dynarray_destroy(&device_extension_properties);
        dynarray_destroy(&device_extension_names);

        return true;
    }


    b8 renderer_shutdown()
    {
        vkDestroyInstance(state.instance, &state.allocator);

        return true;
    }


    static b8 check_validation_layer_support(dynarray_s<const char*>* validation_layers)
    {
        u32 layer_count;
        vkEnumerateInstanceLayerProperties(&layer_count, nullptr);
        dynarray_s<VkLayerProperties> available_layers = dynarray_create<VkLayerProperties>(layer_count);
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


    static b8 select_physical_device(const vulkan_physical_device_requirements_s* requirements)
    {
        fprintf(stderr, "[1] Start selection of physical device.\n");

        u32 device_count = 0;
        vkEnumeratePhysicalDevices(state.instance, &device_count, nullptr);
        if (device_count == 0)
        {
            fprintf(stderr, "No Vulkan-compatible physical devices found.\n");
            return false;
        }
        dynarray_s<VkPhysicalDevice> physical_devices = dynarray_create<VkPhysicalDevice>(device_count);
        vkEnumeratePhysicalDevices(state.instance, &device_count, physical_devices.data);
       
        for (s32 device_index = 0; device_index < device_count; ++device_index)
        {
            // Physical device properties.
            VkPhysicalDeviceDriverProperties driver_properties = { .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DRIVER_PROPERTIES };
            VkPhysicalDeviceProperties2 properties2 = { .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2,
                                                        .pNext = &driver_properties };

            vkGetPhysicalDeviceProperties2(physical_devices.data[device_index], &properties2);
            VkPhysicalDeviceProperties properties = properties2.properties;

            // Physical device features.
            VkPhysicalDeviceFeatures features;
            vkGetPhysicalDeviceFeatures(physical_devices.data[device_index], &features);

            VkPhysicalDeviceLineRasterizationFeaturesEXT smooth_line = { .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_LINE_RASTERIZATION_FEATURES_EXT };
            VkPhysicalDeviceExtendedDynamicStateFeaturesEXT dynamic_state = { .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_FEATURES_EXT,
                                                                              .pNext = &smooth_line };
            VkPhysicalDeviceFeatures2 features2 =  { .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
                                                     .pNext = &dynamic_state };
            
            vkGetPhysicalDeviceFeatures2(physical_devices.data[device_index], &features2);
            
            // Physical device memory properties.
            VkPhysicalDeviceMemoryProperties memory = {};
            vkGetPhysicalDeviceMemoryProperties(physical_devices.data[device_index], &memory);
            

            fprintf(stderr, "Evaluating device: '%s', index %u.", properties.deviceName, device_index);

            vulkan_queue_family_indices_s queue_family_indices = {};
            b8 is_suitable = physical_device_is_suitable(physical_devices.data[device_index], 
                    &properties, &features, requirements, &queue_family_indices);

            if (is_suitable)
            {
                fprintf(stderr, "Selected device: %s\n", properties.deviceName);
                switch (properties.deviceType) 
                {
                    default:
                    case VK_PHYSICAL_DEVICE_TYPE_OTHER:
                    {
                        fprintf(stderr, "GPU type is Unknown.\n");
                    } break;
                    case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
                    {   
                        fprintf(stderr, "GPU type is Integrated GPU.\n");
                    } break;
                    case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
                    {
                        fprintf(stderr, "GPU type is Discrete GPU.\n");
                    } break;
                    case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
                    {
                        fprintf(stderr, "GPU type is Virtual GPU.\n");
                    } break;
                    case VK_PHYSICAL_DEVICE_TYPE_CPU:
                    {
                        fprintf(stderr, "GPU type is CPU.\n");
                    } break;
                }

                // Save off the device-supported API version.
                state.api_major = VK_VERSION_MAJOR(properties.apiVersion);;
                state.api_minor = VK_VERSION_MINOR(properties.apiVersion);;
                state.api_patch = VK_VERSION_PATCH(properties.apiVersion);;

                // Vulkan API version.
                fprintf(stderr,
                    "Vulkan API version: %d.%d.%d",
                    state.api_major,
                    state.api_minor,
                    state.api_minor);

                for (u32 heap_index = 0; heap_index < memory.memoryHeapCount; ++heap_index) 
                {
                    f32 memory_size_gib = (((f32)memory.memoryHeaps[heap_index].size) / 1024.0f / 1024.0f / 1024.0f);
                    if (memory.memoryHeaps[heap_index].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT) 
                    {
                        fprintf(stderr, "Local GPU memory: %.2f GiB", memory_size_gib);
                    } 
                    else 
                    {
                        fprintf(stderr, "Shared System memory: %.2f GiB", memory_size_gib);
                    }
                }
    
                state.physical_device = physical_devices.data[device_index];
                state.queue_family_indices = queue_family_indices;
                state.physical_device_properties = properties;
                state.physical_device_features = features;
                state.physical_device_memory_properties = memory;
                
                state.supports_dynamic_state_natively = false;
                state.supports_dynamic_state = false;
                state.supports_smooth_lines = false;

                // The device may or may not support dynamic state, so save that here.
                if (state.api_major >= 1 && state.api_minor > 2) 
                {
                    state.supports_dynamic_state_natively = true;
                }
                // If not supported natively, it might be supported via extension.
                if (dynamic_state.extendedDynamicState) 
                {
                    state.supports_dynamic_state = true;
                }
                // Check for smooth line rasterization support.
                if (smooth_line.smoothLines) 
                {
                    state.supports_smooth_lines = true;
                }

                break;
            }
        }
        
        dynarray_destroy(&physical_devices);
        if (state.physical_device == VK_NULL_HANDLE)
        {
            fprintf(stderr, "No suitable Vulkan physical device found.\n");
            return false;
        }

        fprintf(stderr, "Physical device selected.\n"); 
        return true;
    }


    static b8 physical_device_is_suitable(VkPhysicalDevice physical_device,
                                          const VkPhysicalDeviceProperties* properties,
                                          const VkPhysicalDeviceFeatures* features,
                                          const vulkan_physical_device_requirements_s* requirements,
                                          vulkan_queue_family_indices_s* out_queue_family_indices)
    {
        out_queue_family_indices->graphics_queue_index = -1;
        out_queue_family_indices->present_queue_index = -1;
        out_queue_family_indices->compute_queue_index = -1;
        out_queue_family_indices->transfer_queue_index = -1;

        if (requirements->discrete_gpu)
        {
            if (properties->deviceType != VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
            {
                fprintf(stderr, "Skipping device: discrete GPU required, but not found.\n");
                return false;
            }
        }

        u32 queue_family_count = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, nullptr);
        dynarray_s<VkQueueFamilyProperties> queue_properties = dynarray_empty<VkQueueFamilyProperties>();
        if (queue_family_count > 0)
        {
            queue_properties = dynarray_create<VkQueueFamilyProperties>(queue_family_count);
            vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, queue_properties.data);
        }

        fprintf(stderr, "Graphics | Present | Compute | Transfer | Name\n");
        u8 min_transfer_score = 255;
        for (s32 queue_family_index = 0; queue_family_index < queue_family_count; ++queue_family_index)
        {
            u8 current_transfer_score = 0;
            
            // graphics queue?
            if (out_queue_family_indices->graphics_queue_index == -1 &&
                queue_properties.data[queue_family_index].queueFlags & VK_QUEUE_GRAPHICS_BIT)
            {
                out_queue_family_indices->graphics_queue_index = queue_family_index;
                current_transfer_score++;
            
                b8 supports_present = vulkan_platform_presentation_support(physical_device, queue_family_index);
                if (supports_present)
                {
                    out_queue_family_indices->present_queue_index = queue_family_index;
                    current_transfer_score++;
                }
            }

            // compute queue?
            if (queue_properties.data[queue_family_index].queueFlags & VK_QUEUE_COMPUTE_BIT)
            {
                out_queue_family_indices->compute_queue_index = queue_family_index;
                current_transfer_score++;
            }

            // transfer queue?
            if (queue_properties.data[queue_family_index].queueFlags & VK_QUEUE_TRANSFER_BIT)
            {
                if (current_transfer_score <= min_transfer_score)
                {
                    min_transfer_score = current_transfer_score;
                    out_queue_family_indices->transfer_queue_index = queue_family_index;
                }
            }
        }

        if (out_queue_family_indices->present_queue_index == -1)
        {
            for (u32 queue_family_index = 0; queue_family_index < queue_family_count; ++queue_family_index)
            {
                b8 supports_present = vulkan_platform_presentation_support(physical_device, queue_family_index);
                if (supports_present)
                {
                    out_queue_family_indices->present_queue_index = queue_family_index;

                    if (out_queue_family_indices->present_queue_index != out_queue_family_indices->graphics_queue_index)
                    {
                        fprintf(stderr, "Warning: Present and graphics operations use different queue families.");
                    }
                }
            }
        }

        fprintf(stderr, "       %d |       %d |       %d |        %d | %s\n",
                out_queue_family_indices->graphics_queue_index != -1,
                out_queue_family_indices->present_queue_index != -1,
                out_queue_family_indices->compute_queue_index != -1,
                out_queue_family_indices->transfer_queue_index != -1,
                properties->deviceName);

        if ((!requirements->graphics || (requirements->graphics && out_queue_family_indices->graphics_queue_index != -1)) &&
            (!requirements->present || (requirements->present && out_queue_family_indices->present_queue_index != -1)) &&
            (!requirements->compute || (requirements->compute && out_queue_family_indices->compute_queue_index != -1)) &&
            (!requirements->transfer || (requirements->transfer && out_queue_family_indices->transfer_queue_index != -1))) 
        {
            fprintf(stderr, "[info] Device meets queue requirements.\n");

            fprintf(stderr, "Graphics queue family index: %d\n", out_queue_family_indices->graphics_queue_index);
            fprintf(stderr, "Present  queue family index: %d\n", out_queue_family_indices->present_queue_index);
            fprintf(stderr, "Transfer queue family index: %d\n", out_queue_family_indices->transfer_queue_index);
            fprintf(stderr, "Compute  queue family index: %d\n", out_queue_family_indices->compute_queue_index);

            if (requirements->device_extension_names.data)
            {
                u32 available_extension_count = 0;
                dynarray_s<VkExtensionProperties> available_extensions = dynarray_empty<VkExtensionProperties>();
                vulkan_eval_result(vkEnumerateDeviceExtensionProperties(physical_device, 0, &available_extension_count, nullptr));
                if (available_extension_count > 0)
                {
                    available_extensions = dynarray_create<VkExtensionProperties>(available_extension_count);
                    vulkan_eval_result(vkEnumerateDeviceExtensionProperties(physical_device, 0, &available_extension_count, available_extensions.data));

                    for (u32 requirement_index = 0; requirement_index < requirements->device_extension_names.capacity; ++requirement_index) 
                    {
                        b8 found = false;
                        for (u32 available_index = 0; available_index < available_extension_count; ++available_index) 
                        {
                            // TODO: string compare 
                            if (strings_equal(requirements->device_extension_names.data[requirement_index], available_extensions[available_index].extensionName)) 
                            {
                                found = true;
                                break;
                            }
                        }
    
                        if (!found) 
                        {
                            fprintf(stderr, "Required extension not found: '%s', skipping device.", requirements->device_extension_names.data[requirement_index]);
                            dynarray_destroy(&available_extensions);
                            return false;
                        }
                    }
                }
                dynarray_destroy(&available_extensions);
            }
        }

        if (requirements->sampler_anisotropy && !features->samplerAnisotropy)
        {
            fprintf(stderr, "Device does not support samplerAnisotropy, skipping.");
            return false;
        }

        if (queue_family_count > 0)
        {
            dynarray_destroy(&queue_properties);
        }

        return true;
    }
}

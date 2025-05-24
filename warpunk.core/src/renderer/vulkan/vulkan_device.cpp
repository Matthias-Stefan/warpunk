#include "warpunk.core/src/renderer/vulkan/vulkan_device.h"
#include "warpunk.core/src/renderer/platform/vulkan_platform.h"

namespace vulkan_renderer
{
    typedef struct vulkan_physical_device_requirements
    {
        b8 graphics;
        b8 present;
        b8 compute;
        b8 transfer;
    
        dynarray<const char*> device_extension_names;
        b8 sampler_anisotropy;
        b8 discrete_gpu;
    } vulkan_physical_device_requirements;

    typedef struct vulkan_queue_family_indices
    { 
        s32 graphics_queue_index;
        s32 present_queue_index;
        s32 compute_queue_index;
        s32 transfer_queue_index;
    } vulkan_queue_family_indices;

    static b8 vulkan_select_physical_device(vulkan_context* context);
    static b8 physical_device_meets_requirements(
        vulkan_context* context,
        VkPhysicalDevice device,
        const VkPhysicalDeviceProperties* properties,
        const VkPhysicalDeviceFeatures* features,
        const vulkan_physical_device_requirements* requirements,
        vulkan_queue_family_indices* out_queue_family_info,
        vulkan_swapchain_support_info* out_swapchain_support);

    b8 vulkan_device_create(vulkan_context* context)
    {
        if (!vulkan_select_physical_device(context)) 
        {
            WERROR("Failed to select a suitable Vulkan physical device.");
            return false;
        }

        // Logical Device & Queues
        WINFO("Creating logical device...");
        b8 present_shares_graphics_queue = context->device.graphics_queue_index == context->device.present_queue_index;
        b8 transfer_shares_graphics_queue = context->device.graphics_queue_index == context->device.transfer_queue_index;
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
        queue_indices[queue_index++] = context->device.graphics_queue_index;
        if (!present_shares_graphics_queue)
        {
            queue_indices[queue_index++] = context->device.present_queue_index;
        }
        if (!transfer_shares_graphics_queue)
        {
            queue_indices[queue_index++] = context->device.transfer_queue_index;
        }

        dynarray<VkDeviceQueueCreateInfo> queue_create_info = dynarray_create<VkDeviceQueueCreateInfo>(index_count);        
        f32 queue_priorities[2] = { 0.9f, 1.0f };
       
        VkQueueFamilyProperties props[64];
        u32 prop_count;
        vkGetPhysicalDeviceQueueFamilyProperties(context->device.physical_device, &prop_count, nullptr);
        vkGetPhysicalDeviceQueueFamilyProperties(context->device.physical_device, &prop_count, props);

        for (u32 i = 0; i < index_count; ++i) {
            queue_create_info.data[i].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queue_create_info.data[i].queueFamilyIndex = queue_indices[i];
            queue_create_info.data[i].queueCount = 1;
    
            if (present_shares_graphics_queue && queue_indices[i] == context->device.present_queue_index) 
            {
                if (props[context->device.present_queue_index].queueCount > 1) 
                {
                    queue_create_info.data[i].queueCount = 2;
                } 
                else 
                {
                    present_must_share_graphics = true;
                    // HACK:
                    (void)present_must_share_graphics;
                }
            }
    
            if (queue_indices[i] == context->device.graphics_queue_index) 
            {
                 queue_create_info.data[i].queueCount = 2;
            }
            queue_create_info.data[i].flags = 0;
            queue_create_info.data[i].pNext = 0;
            queue_create_info.data[i].pQueuePriorities = queue_priorities;
        }  

        // Device extension
        u32 available_device_extension_count = 0;
        vulkan_eval_result(vkEnumerateDeviceExtensionProperties(context->device.physical_device, nullptr, &available_device_extension_count, nullptr));
        dynarray<VkExtensionProperties> device_extension_properties = dynarray_empty<VkExtensionProperties>();
        if (available_device_extension_count > 0)
        {
            device_extension_properties = dynarray_create<VkExtensionProperties>(available_device_extension_count); 
            vulkan_eval_result(vkEnumerateDeviceExtensionProperties(context->device.physical_device, nullptr, &available_device_extension_count, device_extension_properties.data));
        }

        dynarray<const char*> device_extension_names = dynarray_empty<const char *>();
        dynarray_add(&device_extension_names, VK_KHR_SWAPCHAIN_EXTENSION_NAME);

        // If native support for dynamic state is missing but the extension is available,
        // enable the required extensions for dynamic state and dynamic rendering.
        if (!context->device.supports_dynamic_state_natively && context->device.supports_dynamic_state)
        {
            dynarray_add(&device_extension_names, VK_EXT_EXTENDED_DYNAMIC_STATE_EXTENSION_NAME);
            dynarray_add(&device_extension_names, VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
        }
        // If the device supports smooth (antialiased) lines, enable the line rasterization extension.
        if (context->device.supports_smooth_lines)
        {
            dynarray_add(&device_extension_names, VK_EXT_LINE_RASTERIZATION_EXTENSION_NAME);
        }

        // Set up the device features to request, based on what the physical device supports.
        VkPhysicalDeviceFeatures2 device_features2 = {};
        device_features2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2_KHR;
        device_features2.pNext = nullptr;
        // Enable desired features if supported.
        device_features2.features.samplerAnisotropy = context->device.physical_device_features.samplerAnisotropy;
        device_features2.features.fillModeNonSolid = context->device.physical_device_features.fillModeNonSolid;
        device_features2.features.shaderClipDistance = context->device.physical_device_features.shaderClipDistance;
        // Ensure that required features are actually supported before proceeding.
        if (!device_features2.features.shaderClipDistance)
        {
            WERROR("shaderClipDistance not supported by device.");
            return false;
        }
        
        // Dynamic rendering.
        VkPhysicalDeviceDynamicRenderingFeatures dynamic_rendering_ext = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES };
        dynamic_rendering_ext.dynamicRendering = VK_TRUE;
        device_features2.pNext = &dynamic_rendering_ext;

        // VK_EXT_extended_dynamic_state
        VkPhysicalDeviceExtendedDynamicStateFeaturesEXT extended_dynamic_state = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_FEATURES_EXT };
        extended_dynamic_state.extendedDynamicState = VK_TRUE;
        dynamic_rendering_ext.pNext = &extended_dynamic_state;

        // VK_EXT_descriptor_indexing
        VkPhysicalDeviceDescriptorIndexingFeatures descriptor_indexing_features = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES_EXT };
        descriptor_indexing_features.descriptorBindingPartiallyBound = VK_FALSE; // Don't use this.
        extended_dynamic_state.pNext = &descriptor_indexing_features;

        // Smooth line rasterisation, if supported.
        VkPhysicalDeviceLineRasterizationFeaturesEXT line_rasterization_ext = {};
        if (context->device.supports_smooth_lines)
        {
            line_rasterization_ext.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_LINE_RASTERIZATION_FEATURES_EXT;
            line_rasterization_ext.smoothLines = VK_TRUE;
            descriptor_indexing_features.pNext = &line_rasterization_ext;
        }

        VkDeviceCreateInfo device_create_info = {};
        device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        device_create_info.queueCreateInfoCount = index_count;
        device_create_info.pQueueCreateInfos = queue_create_info.data;
        device_create_info.pEnabledFeatures = 0;
        device_create_info.enabledExtensionCount = device_extension_names.capacity;
        device_create_info.ppEnabledExtensionNames = device_extension_names.data;
        
        device_create_info.enabledLayerCount = 0;
        device_create_info.ppEnabledLayerNames = 0;
        device_create_info.pNext = &device_features2; 

        vulkan_eval_result(
                vkCreateDevice(
                    context->device.physical_device, &device_create_info, NULL, &context->device.logical_device)); 

        dynarray_destroy(&queue_create_info);
        dynarray_destroy(&device_extension_properties);
        dynarray_destroy(&device_extension_names);

        WSUCCESS("logical device created.");

        vkGetDeviceQueue(context->device.logical_device, 
                         context->device.graphics_queue_index, 
                         0, 
                         &context->device.graphics_queue);

        vkGetDeviceQueue(context->device.logical_device, 
                         context->device.present_queue_index, 
                         present_shares_graphics_queue ? 0 : 
                            (context->device.graphics_queue_index == context->device.present_queue_index) ? 1 : 0, 
                         &context->device.present_queue);

        vkGetDeviceQueue(context->device.logical_device, 
                         context->device.transfer_queue_index, 
                         0, 
                         &context->device.transfer_queue);

        WSUCCESS("queues obtained.");

        VkCommandPoolCreateInfo command_pool_create_info = { 
            .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
            .pNext = nullptr,
            .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
            .queueFamilyIndex = static_cast<u32>(context->device.graphics_queue_index)
        };

        vulkan_eval_result(vkCreateCommandPool(
                    context->device.logical_device, 
                    &command_pool_create_info, 
                    0,
                    &context->graphics_command_pool));

        WSUCCESS("graphics command pool created.");

        return true;
    }

    void vulkan_device_destroy(vulkan_context* context)
    {

    }

    void vulkan_device_query_swapchain_support(
        vulkan_context* context, 
        VkPhysicalDevice physical_device,
        VkSurfaceKHR surface, 
        vulkan_swapchain_support_info* out_support_info)
    {

    }

    b8 vulkan_device_detect_depth_format(vulkan_context* context, vulkan_device* device)
    {
        return true;
    }

    static b8 vulkan_select_physical_device(vulkan_context* context)
    {
        vulkan_device* device = &context->device;

        vulkan_physical_device_requirements requirements = {};
        requirements.graphics = true;
        requirements.present = true;
        requirements.compute = true;
        requirements.transfer = true;
        requirements.device_extension_names = dynarray_empty<const char*>();
        dynarray_add(&requirements.device_extension_names, VK_KHR_SWAPCHAIN_EXTENSION_NAME);
        requirements.sampler_anisotropy = true;
        requirements.discrete_gpu = true;

        WINFO("[1] Start selection of physical device.");

        u32 device_count = 0;
        vkEnumeratePhysicalDevices(context->instance, &device_count, nullptr);
        if (device_count == 0)
        {
            WERROR("No Vulkan-compatible physical devices found.");
            return false;
        }
        dynarray<VkPhysicalDevice> physical_devices = dynarray_create<VkPhysicalDevice>(device_count);
        vkEnumeratePhysicalDevices(context->instance, &device_count, physical_devices.data);
       
        for (u32 device_index = 0; device_index < device_count; ++device_index)
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
            
            WINFO("Evaluating device: '%s', index %u.", properties.deviceName, device_index);

            vulkan_queue_family_indices queue_family_indices = {};
            vulkan_swapchain_support_info swapchain_support = {};
            b8 is_suitable = physical_device_meets_requirements(context, physical_devices.data[device_index], 
                    &properties, &features, &requirements, &queue_family_indices, &context->device.swapchain_support);

            if (is_suitable)
            {
                WINFO("Selected device: %s.", properties.deviceName);
                switch (properties.deviceType) 
                {
                    default:
                    case VK_PHYSICAL_DEVICE_TYPE_OTHER:
                    {
                        WWARNING("GPU type is Unknown.");
                    } break;
                    case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
                    {   
                        WINFO("GPU type is Integrated GPU.");
                    } break;
                    case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
                    {
                        WINFO("GPU type is Discrete GPU.");
                    } break;
                    case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
                    {
                        WINFO("GPU type is Virtual GPU.");
                    } break;
                    case VK_PHYSICAL_DEVICE_TYPE_CPU:
                    {
                        WINFO("GPU type is CPU.");
                    } break;
                }

                // Save off the device-supported API version.
                device->api_major = VK_VERSION_MAJOR(properties.apiVersion);;
                device->api_minor = VK_VERSION_MINOR(properties.apiVersion);;
                device->api_patch = VK_VERSION_PATCH(properties.apiVersion);;

                // Vulkan API version.
                WINFO("Vulkan API version: %d.%d.%d",
                      device->api_major,
                      device->api_minor,
                      device->api_minor);

                for (u32 heap_index = 0; heap_index < memory.memoryHeapCount; ++heap_index) 
                {
                    f32 memory_size_gib = (((f32)memory.memoryHeaps[heap_index].size) / 1024.0f / 1024.0f / 1024.0f);
                    if (memory.memoryHeaps[heap_index].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT) 
                    {
                        WINFO("Local GPU memory: %.2f GiB", memory_size_gib);
                    } 
                    else 
                    {
                        WINFO("Shared System memory: %.2f GiB", memory_size_gib);
                    }
                }
    
                device->physical_device = physical_devices.data[device_index];
                device->graphics_queue_index = queue_family_indices.graphics_queue_index;
                device->present_queue_index = queue_family_indices.present_queue_index;
                device->compute_queue_index = queue_family_indices.compute_queue_index;
                device->transfer_queue_index = queue_family_indices.transfer_queue_index;

                device->physical_device_properties = properties;
                device->physical_device_features = features;
                device->physical_device_memory_properties = memory;
                
                device->supports_dynamic_state_natively = false;
                device->supports_dynamic_state = false;
                device->supports_smooth_lines = false;

                // The device may or may not support dynamic state, so save that here.
                if (device->api_major >= 1 && device->api_minor > 2) 
                {
                    device->supports_dynamic_state_natively = true;
                }
                // If not supported natively, it might be supported via extension.
                if (dynamic_state.extendedDynamicState) 
                {
                    device->supports_dynamic_state = true;
                }
                // Check for smooth line rasterization support.
                if (smooth_line.smoothLines) 
                {
                    device->supports_smooth_lines = true;
                }

                break;
            }
        }
        
        dynarray_destroy(&physical_devices);
        if (context->device.physical_device == VK_NULL_HANDLE)
        {
            WERROR("No suitable Vulkan physical device found.");
            return false;
        }

        WSUCCESS("Physical device selected."); 
        return true;
    }
    
    static b8 physical_device_meets_requirements(
        vulkan_context* context,
        VkPhysicalDevice device,
        const VkPhysicalDeviceProperties* properties,
        const VkPhysicalDeviceFeatures* features,
        const vulkan_physical_device_requirements* requirements,
        vulkan_queue_family_indices* out_queue_family_info,
        vulkan_swapchain_support_info* out_swapchain_support)
    {
        out_queue_family_info->graphics_queue_index = -1;
        out_queue_family_info->present_queue_index = -1;
        out_queue_family_info->compute_queue_index = -1;
        out_queue_family_info->transfer_queue_index = -1;

        if (requirements->discrete_gpu)
        {
            if (properties->deviceType != VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
            {
                WWARNING("Skipping device: discrete GPU required, but not found.");
                return false;
            }
        }

        u32 queue_family_count = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, nullptr);
        dynarray<VkQueueFamilyProperties> queue_properties = dynarray_empty<VkQueueFamilyProperties>();
        if (queue_family_count > 0)
        {
            queue_properties = dynarray_create<VkQueueFamilyProperties>(queue_family_count);
            vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, queue_properties.data);
        }

        WINFO("Graphics | Present | Compute | Transfer | Name");
        u8 min_transfer_score = 255;
        for (u32 queue_family_index = 0; queue_family_index < queue_family_count; ++queue_family_index)
        {
            u8 current_transfer_score = 0;
            
            // graphics queue?
            if (out_queue_family_info->graphics_queue_index == -1 &&
                queue_properties.data[queue_family_index].queueFlags & VK_QUEUE_GRAPHICS_BIT)
            {
                out_queue_family_info->graphics_queue_index = queue_family_index;
                current_transfer_score++;
            
                b8 supports_present = vulkan_platform_presentation_support(device, queue_family_index);
                if (supports_present)
                {
                    out_queue_family_info->present_queue_index = queue_family_index;
                    current_transfer_score++;
                }
            }

            // compute queue?
            if (queue_properties.data[queue_family_index].queueFlags & VK_QUEUE_COMPUTE_BIT)
            {
                out_queue_family_info->compute_queue_index = queue_family_index;
                current_transfer_score++;
            }

            // transfer queue?
            if (queue_properties.data[queue_family_index].queueFlags & VK_QUEUE_TRANSFER_BIT)
            {
                if (current_transfer_score <= min_transfer_score)
                {
                    min_transfer_score = current_transfer_score;
                    out_queue_family_info->transfer_queue_index = queue_family_index;
                }
            }
        }

        if (out_queue_family_info->present_queue_index == -1)
        {
            for (u32 queue_family_index = 0; queue_family_index < queue_family_count; ++queue_family_index)
            {
                b8 supports_present = vulkan_platform_presentation_support(device, queue_family_index);
                if (supports_present)
                {
                    out_queue_family_info->present_queue_index = queue_family_index;

                    if (out_queue_family_info->present_queue_index != out_queue_family_info->graphics_queue_index)
                    {
                        WWARNING("Present and graphics operations use different queue families.");
                    }
                }
            }
        }

        WINFO("       %d |       %d |       %d |        %d | %s",
              out_queue_family_info->graphics_queue_index != -1,
              out_queue_family_info->present_queue_index != -1,
              out_queue_family_info->compute_queue_index != -1,
              out_queue_family_info->transfer_queue_index != -1,
              properties->deviceName);

        if ((!requirements->graphics || (requirements->graphics && out_queue_family_info->graphics_queue_index != -1)) &&
            (!requirements->present || (requirements->present && out_queue_family_info->present_queue_index != -1)) &&
            (!requirements->compute || (requirements->compute && out_queue_family_info->compute_queue_index != -1)) &&
            (!requirements->transfer || (requirements->transfer && out_queue_family_info->transfer_queue_index != -1))) 
        {
            WINFO("Device meets queue requirements.");

            WINFO("Graphics queue family index: %d", out_queue_family_info->graphics_queue_index);
            WINFO("Present  queue family index: %d", out_queue_family_info->present_queue_index);
            WINFO("Transfer queue family index: %d", out_queue_family_info->transfer_queue_index);
            WINFO("Compute  queue family index: %d", out_queue_family_info->compute_queue_index);

            if (requirements->device_extension_names.data)
            {
                u32 available_extension_count = 0;
                dynarray<VkExtensionProperties> available_extensions = dynarray_empty<VkExtensionProperties>();
                vulkan_eval_result(vkEnumerateDeviceExtensionProperties(device, 0, &available_extension_count, nullptr));
                if (available_extension_count > 0)
                {
                    available_extensions = dynarray_create<VkExtensionProperties>(available_extension_count);
                    vulkan_eval_result(vkEnumerateDeviceExtensionProperties(device, 0, &available_extension_count, available_extensions.data));

                    for (u32 requirement_index = 0; requirement_index < requirements->device_extension_names.capacity; ++requirement_index) 
                    {
                        b8 found = false;
                        for (u32 available_index = 0; available_index < available_extension_count; ++available_index) 
                        {
                            if (strcmp(requirements->device_extension_names.data[requirement_index], available_extensions.data[available_index].extensionName) == 0) 
                            {
                                found = true;
                                break;
                            }
                        }
    
                        if (!found) 
                        {
                            WERROR("Required extension not found: '%s', skipping device.", requirements->device_extension_names.data[requirement_index]);
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
            WERROR("Device does not support samplerAnisotropy, skipping.");
            return false;
        }

        if (queue_family_count > 0)
        {
            dynarray_destroy(&queue_properties);
        }

        return true;
    }
}
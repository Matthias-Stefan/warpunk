#include "warpunk.core/src/renderer/renderer_backend.h"

#include <string.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

#include "warpunk.core/src/container/dynarray.hpp"
#include "warpunk.core/src/renderer/vulkan/vulkan_types.h"
#include "warpunk.core/src/renderer/platform/vulkan_platform.h"
#include "warpunk.core/src/renderer/vulkan/vulkan_device.h"
#include "warpunk.core/src/renderer/vulkan/vulkan_swapchain.h"

namespace vulkan_renderer 
{
    static b8 vulkan_allocator_create(vulkan_context* context, VkAllocationCallbacks* callbacks);
    static void* vulkan_memory_alloc(void* user_data, size_t size, size_t alignment, VkSystemAllocationScope allocation_scope);
    static void* vulkan_memory_realloc(void* user_data, void* original, size_t size, size_t alignment, VkSystemAllocationScope allocation_scope);
    static void vulkan_memory_free(void* user_data, void* memory);
    static void vulkan_memory_alloc_notification(void* user_data, size_t size, VkInternalAllocationType allocation_type, VkSystemAllocationScope allocation_scope);
    static void vulkan_memory_free_notification(void* user_data, size_t size, VkInternalAllocationType allocation_type, VkSystemAllocationScope allocation_scope);

    static b8 vulkan_check_validation_layer_support(dynarray<const char*>* validation_layers);

    static vulkan_context context;
    // TODO: move
    static warpunk_window_renderer_backend_state window_renderer_backend_state;

    b8 renderer_startup(renderer_config renderer_config)
    {
        if (context.is_initialized)
        {
            return true;
        }
        context.config_flags = renderer_config.flags;

        // Custom allocator
        context.allocator = (VkAllocationCallbacks *)platform_memory_alloc(sizeof(VkAllocationCallbacks));
        if (!vulkan_allocator_create(&context, context.allocator))
        {
            platform_memory_free(context.allocator);
            WERROR("Failed to create Vulkan memory allocator. Continuing using the driver's default allocator.");
            context.allocator = 0;
        }

        // Query the supported Vulkan API version and store the major, minor, and patch components in the context.
        u32 api_version = 0;
        vkEnumerateInstanceVersion(&api_version);
        context.api_major = VK_VERSION_MAJOR(api_version);
        context.api_minor = VK_VERSION_MINOR(api_version);
        context.api_patch = VK_VERSION_PATCH(api_version);

        // Instance
        VkApplicationInfo application_info = {};
        application_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        application_info.pApplicationName = renderer_config.application_name;
        application_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        application_info.pEngineName = "Warpunk Engine";
        application_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        application_info.apiVersion = VK_API_VERSION_1_3;

        VkInstanceCreateInfo instance_create_info = {};
        instance_create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        instance_create_info.pApplicationInfo = &application_info;

        // Extensions
        u32 instance_extension_count = 0;
        vulkan_platform_get_required_instance_extensions(&instance_extension_count, nullptr);
        dynarray<const char *> instance_extensions = dynarray_create<const char *>(instance_extension_count);
        vulkan_platform_get_required_instance_extensions(&instance_extension_count, &instance_extensions.data);

        dynarray_add(&instance_extensions, "VK_EXT_debug_utils");
        // TODO: verfiy
        instance_extension_count += 1;

        instance_create_info.enabledLayerCount = 0;
        instance_create_info.ppEnabledLayerNames = nullptr;
        instance_create_info.enabledExtensionCount = instance_extension_count;
        instance_create_info.ppEnabledExtensionNames = instance_extensions.data;

        // validation layers

        dynarray<const char*> validation_layers = dynarray_empty<const char *>();
        dynarray_add(&validation_layers, "VK_LAYER_KHRONOS_validation");
#ifdef WARPUNK_DEBUG
        const b8 enable_validation_layers = true;
#else
        const b8 enable_validation_layers = false;    
#endif
        if (enable_validation_layers && !vulkan_check_validation_layer_support(&validation_layers))
        {
            return false;
        }

        if (enable_validation_layers)
        {
            instance_create_info.enabledLayerCount = validation_layers.capacity;
            instance_create_info.ppEnabledLayerNames = validation_layers.data;
        }

        vulkan_eval_result(vkCreateInstance(&instance_create_info, nullptr, &context.instance));
        dynarray_destroy(&instance_extensions);
        dynarray_destroy(&validation_layers);

        WSUCCESS("Vulkan Instance created.");

        // TODO: Wait until the platform has created a window
        // surface
        if (!vulkan_platform_create_surface(context.instance, NULL, &context.surface))
        {
            WERROR("Failed to create surface.");
            return false;
        }
        
        // Device creation
        if (!vulkan_device_create(&context)) 
        {
            WERROR("Failed to create device!");
            return false;
        }

        // TODO:
#if false
        // Samplers array.
        context->samplers = darray_create(vulkan_sampler_handle_data);

        // Shaders array.
        context->shaders = darray_reserve(vulkan_shader, config->max_shader_count);

        // Create a shader compiler to be used.
        context->shader_compiler = shaderc_compiler_initialize();

        WINFO("Renderer config requests %s-buffering to be used.", config->use_triple_buffering ? "triple" : "double");
        context->triple_buffering_enabled = config->use_triple_buffering;

#endif
        WSUCCESS("Vulkan renderer initialized successfully.");

        // LAST:
        // ===========================================================================================================
        // TODO: Wait until the platform has created a window
        window_renderer_backend_state = {};
        window_renderer_backend_state.surface = context.surface;
        // TODO: HACK: Code bright like a diamond
        if (!vulkan_swapchain_create(
                &context,
                960, 
                540,
                &window_renderer_backend_state.swapchain))
        {
            WERROR("Failed to create swapchain!");
            return false;
        }

        return true;
    }


    b8 renderer_shutdown()
    {
        vkDestroyInstance(context.instance, NULL);

        return true;
    }


    void renderer_device_query_swapchain_support(vulkan_swapchain_support_info* out_swapchain_support)
    {
        // surface capabilities
        vulkan_eval_result(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(context.device.physical_device, context.surface, &out_swapchain_support->capabilities));        
        
        // surface formats
        vulkan_eval_result(vkGetPhysicalDeviceSurfaceFormatsKHR(context.device.physical_device, context.surface, &out_swapchain_support->format_count, 0));
        if (out_swapchain_support->format_count != 0)
        {
            if (!out_swapchain_support->formats)
            {
                out_swapchain_support->formats = (VkSurfaceFormatKHR *)platform_memory_alloc(sizeof(VkSurfaceFormatKHR) * out_swapchain_support->format_count);
            }

            vulkan_eval_result(vkGetPhysicalDeviceSurfaceFormatsKHR(context.device.physical_device, context.surface, &out_swapchain_support->format_count, out_swapchain_support->formats));
        }

        // present modes
        vulkan_eval_result(vkGetPhysicalDeviceSurfacePresentModesKHR(context.device.physical_device, context.surface, &out_swapchain_support->present_mode_count, 0));
        if (out_swapchain_support->present_mode_count != 0)
        {
            if (!out_swapchain_support->present_modes)
            {
                out_swapchain_support->present_modes = (VkPresentModeKHR *)platform_memory_alloc(sizeof(VkPresentModeKHR) * out_swapchain_support->present_mode_count);
            }

            vulkan_eval_result(vkGetPhysicalDeviceSurfacePresentModesKHR(context.device.physical_device, context.surface, &out_swapchain_support->present_mode_count, out_swapchain_support->present_modes));
        }
    }

/**
 * =================== VULKAN ALLOCATOR ===================
 */

    static b8 vulkan_allocator_create(vulkan_context* context, VkAllocationCallbacks* callbacks)
    {
        if (context->allocator)
        {
            callbacks->pfnAllocation = vulkan_memory_alloc;
            callbacks->pfnReallocation = vulkan_memory_realloc;
            callbacks->pfnFree = vulkan_memory_free;
            callbacks->pfnInternalAllocation = vulkan_memory_alloc_notification;
            callbacks->pfnInternalFree = vulkan_memory_free_notification;
            callbacks->pUserData = context;
            return true;
        }

        return false;
    }

    static void* vulkan_memory_alloc(void* user_data, size_t size, size_t alignment, VkSystemAllocationScope allocation_scope)
    {
        if (size == 0)
        {
            return nullptr;
        }

        void* result = platform_memory_alloc(size);
        return result;
    }
    
    static void* vulkan_memory_realloc(void* user_data, void* original, size_t size, size_t alignment, VkSystemAllocationScope allocation_scope)
    {
        if (!original)
        {
            return vulkan_memory_alloc(user_data, size, alignment, allocation_scope);
        }

        if (size == 0)
        {
            platform_memory_free(user_data);
            return nullptr;
        }

        void* result = vulkan_memory_alloc(user_data, size, alignment, allocation_scope);
        if (!result)
        {
            WERROR("Failed to realloc %p.", original);
            return nullptr;
        }

        platform_memory_copy(result, original, size);
        return result;
    }
    
    static void vulkan_memory_free(void* user_data, void* memory)
    {
        if (!memory)
        {
            WWARNING("Attempted to free a null memory pointer.");
            return;
        }

        platform_memory_free(memory);
    }
    
    static void vulkan_memory_alloc_notification(void* user_data, size_t size, VkInternalAllocationType allocation_type, VkSystemAllocationScope allocation_scope)
    {
        WVERBOSE("External allocation of size: %llu", size);
    }

    static void vulkan_memory_free_notification(void* user_data, size_t size, VkInternalAllocationType allocation_type, VkSystemAllocationScope allocation_scope)
    {
        WVERBOSE("External free of size: %llu", size);
    }

    static b8 vulkan_check_validation_layer_support(dynarray<const char*>* validation_layers)
    {
        u32 layer_count;
        vkEnumerateInstanceLayerProperties(&layer_count, nullptr);
        dynarray<VkLayerProperties> available_layers = dynarray_create<VkLayerProperties>(layer_count);
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
}

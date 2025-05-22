#include "warpunk.core/src/renderer/vulkan/vulkan_swapchain.h"

#include "warpunk.core/src/math/math_common.hpp"
#include "warpunk.core/src/renderer/vulkan/vulkan_types.h"
#include "warpunk.core/src/renderer/vulkan/vulkan_backend.h"

namespace vulkan_renderer
{
    static b8 create(const vulkan_state_s* const state, vulkan_swapchain_support_info_s* swapchain_support, u32 width, u32 height, vulkan_swapchain_s* out_swapchain);

    b8 vulkan_swapchain_create(const vulkan_state_s* const state,
                               vulkan_swapchain_support_info_s* swapchain_support,
                               u32 width, u32 height,
                               vulkan_swapchain_s* out_swapchain)
    {
        return create(state, swapchain_support, width, height, out_swapchain);
    }

    b8 vulkan_swapchain_recreate()
    {
        return true;
    }

    b8 vulkan_swapchain_destroy()
    {
        return true;
    }

    static b8 create(const vulkan_state_s* const state, vulkan_swapchain_support_info_s* swapchain_support, u32 width, u32 height, vulkan_swapchain_s* out_swapchain)
    {
        renderer_device_query_swapchain_support(swapchain_support);
     
        VkExtent2D swapchain_extent = { width, height };

        b8 found = false;
        for (u32 format_index = 0; format_index < swapchain_support->format_count; ++format_index)
        {
            VkSurfaceFormatKHR surface_format = swapchain_support->formats[format_index];
             
            if (surface_format.format == VK_FORMAT_B8G8R8A8_UNORM && 
                surface_format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
            {
                out_swapchain->image_format = surface_format;
                found = true;
                break;
            }
        }

        if (!found)
        {
            out_swapchain->image_format = swapchain_support->formats[0];
        }

        VkFormatProperties format_properties = {};
        vkGetPhysicalDeviceFormatProperties(state->physical_device, out_swapchain->image_format.format, &format_properties);
        
        out_swapchain->supports_blit_dst = (format_properties.optimalTilingFeatures & VK_FORMAT_FEATURE_BLIT_DST_BIT) != 0;
        out_swapchain->supports_blit_src = (format_properties.optimalTilingFeatures & VK_FORMAT_FEATURE_BLIT_SRC_BIT) != 0;
        fprintf(stderr, "Swapchain image format %s be a blit destination.\n", out_swapchain->supports_blit_dst ? "CAN" : "CANNOT");
        fprintf(stderr, "Swapchain image format %s be a blit source.\n", out_swapchain->supports_blit_src ? "CAN" : "CANNOT");

        VkPresentModeKHR present_mode;
        if (state->config_flags & RENDERER_CONFIG_FLAG_VSYNC_ENABLED_BIT) 
        {
            present_mode = VK_PRESENT_MODE_FIFO_KHR;
            if ((state->config_flags & RENDERER_CONFIG_FLAG_POWER_SAVING_BIT) == 0) 
            {
                for (u32 present_mode_index = 0; 
                     present_mode_index < swapchain_support->present_mode_count; 
                     ++present_mode_index)
                {
                    VkPresentModeKHR mode = swapchain_support->present_modes[present_mode_index];
                    if (mode == VK_PRESENT_MODE_MAILBOX_KHR) 
                    {
                        present_mode = mode;
                        break;
                    }
                }
            }
        } 
        else 
        {
            present_mode = VK_PRESENT_MODE_IMMEDIATE_KHR;
        }

        if (swapchain_support->format_count < 1 || swapchain_support->present_mode_count < 1) 
        {
            if (swapchain_support->formats) 
            {
                platform_memory_free(swapchain_support->formats);
            }
            if (swapchain_support->present_modes) 
            {
                platform_memory_free(swapchain_support->present_modes);
            }

            fprintf(stderr, "Required swapchain support not present, skipping device.\n");
            return false;
        }

        // swapchain extent
        if (state->swapchain_support.capabilities.currentExtent.width != MAX_U32) 
        {
            swapchain_extent = state->swapchain_support.capabilities.currentExtent;
        }

        // Clamp to the value allowed by the GPU.
        VkExtent2D min = state->swapchain_support.capabilities.minImageExtent;
        VkExtent2D max = state->swapchain_support.capabilities.maxImageExtent;
        swapchain_extent.width = clamp<u32>(min.width, max.width, swapchain_extent.width);
        swapchain_extent.height = clamp<u32>(min.height, max.height, swapchain_extent.height);
    
        u32 image_count = state->swapchain_support.capabilities.minImageCount + 1;
        if (state->swapchain_support.capabilities.maxImageCount > 0 && 
                image_count > state->swapchain_support.capabilities.maxImageCount) 
        {
            image_count = state->swapchain_support.capabilities.maxImageCount;
        }
    
        // Swapchain create info
        VkSwapchainCreateInfoKHR swapchain_create_info = {};
        swapchain_create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        swapchain_create_info.surface = state->surface;
        swapchain_create_info.minImageCount = image_count;
        swapchain_create_info.imageFormat = out_swapchain->image_format.format;
        swapchain_create_info.imageColorSpace = out_swapchain->image_format.colorSpace;
        swapchain_create_info.imageExtent = swapchain_extent;
        swapchain_create_info.imageArrayLayers = 1;
        swapchain_create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
  
        // Setup the queue family indices
        if (state->queue_family_indices.graphics_queue_index != state->queue_family_indices.present_queue_index) 
        {
            u32 queueFamilyIndices[] = {
                (u32)state->queue_family_indices.graphics_queue_index,
                (u32)state->queue_family_indices.present_queue_index
            };
            
            swapchain_create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            swapchain_create_info.queueFamilyIndexCount = 2;
            swapchain_create_info.pQueueFamilyIndices = queueFamilyIndices;
        } 
        else 
        {
            swapchain_create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
            swapchain_create_info.queueFamilyIndexCount = 0;
            swapchain_create_info.pQueueFamilyIndices = 0;
        }

        swapchain_create_info.preTransform = state->swapchain_support.capabilities.currentTransform;
        swapchain_create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        swapchain_create_info.presentMode = present_mode;
        swapchain_create_info.clipped = VK_TRUE;
        swapchain_create_info.oldSwapchain = 0;

        vulkan_eval_result(vkCreateSwapchainKHR(state->device, 
                                                &swapchain_create_info, 
                                                nullptr /** context->allocator */, 
                                                &out_swapchain->handle)); 

// TODO:
#if false
        if (!swapchain->swapchain_colour_texture) 
        {
            out_swapchain->swapchain_colour_texture = platform_memory_alloc(sizeof()) KALLOC_TYPE(kresource_texture, MEMORY_TAG_RENDERER);
            out_swapchain->swapchain_colour_texture->renderer_texture_handle = khandle_invalid();
        }
#endif 

        // Get image count from swapchain.
        out_swapchain->image_count = 0;
        vulkan_eval_result(vkGetSwapchainImagesKHR(state->device, out_swapchain->handle, &out_swapchain->image_count, 0));

#if false
    // Get the actual images from swapchain.
    VkImage swapchain_images[32];
    result = rhi->kvkGetSwapchainImagesKHR(context->device.logical_device, swapchain->handle, &swapchain->image_count, swapchain_images);
    if (!vulkan_result_is_success(result)) {
        const char* result_str = vulkan_result_string(result, true);
        KFATAL("Failed to obtain images from Vulkan swapchain with the error: '%s'.", result_str);
        return false;
    }

    // Swapchain images are stored in the backend data of the swapchain's colour texture.
    // NOTE: The window should create a separate set of images to render to, then blit to these.
    if (khandle_is_invalid(swapchain->swapchain_colour_texture->renderer_texture_handle)) {
        // If invalid, then a new one needs to be created. This does not reach out to the
        // texture system to create this, but handles it internally instead. This is because
        // the process for this varies greatly between backends.
        if (!renderer_kresource_texture_resources_acquire(
                backend->frontend_state,
                kname_create("__swapchain_colour_texture__"),
                TEXTURE_TYPE_2D,
                swapchain_extent.width,
                swapchain_extent.height,
                4,
                1,
                1,
                // NOTE: This should be a wrapped texture, so the frontend does not try to
                // acquire the resources we already have here.
                TEXTURE_FLAG_IS_WRAPPED | TEXTURE_FLAG_IS_WRITEABLE | TEXTURE_FLAG_RENDERER_BUFFERING,
                &swapchain->swapchain_colour_texture->renderer_texture_handle)) {

            KFATAL("Failed to acquire internal texture resources for swapchain colour texture.");
            return false;
        }

#endif



        return true; 
    }
}

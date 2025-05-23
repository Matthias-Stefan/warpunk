#include "warpunk.core/src/renderer/renderer_backend.h"
#include "warpunk.core/src/renderer/renderer_backend_software.h"
#include "warpunk.core/src/renderer/vulkan/vulkan_backend.h"

typedef struct renderer_api
{
    b8 (*renderer_startup)(renderer_config renderer_config);
    b8 (*renderer_shutdown)();
    void (*renderer_begin_frame)();
    void (*renderer_end_frame)();
    buffer_handle (*renderer_create_buffer)(s32 size, void* data);
    void (*renderer_destroy_buffer)(buffer_handle buffer_handle);
    texture_handle (*renderer_create_texture)(s32 width, s32 height, void* data);
    void (*renderer_destroy_texture)(texture_handle texture_handle);
    void (*renderer_draw)(void* vertex_array, void* material);
} renderer_api;

static renderer_api api; 

b8 renderer_startup(renderer_config renderer_config)
{
    switch (renderer_config.type)
    {
        case RENDERER_TYPE_SOFTWARE:
        {
            api.renderer_startup = software_renderer::renderer_startup;
            //renderer_api.renderer_shutdown = software_renderer::renderer_shutdown;
            api.renderer_begin_frame = software_renderer::renderer_begin_frame;
            //renderer_api.renderer_end_frame = software_renderer::renderer_end_frame;
            //renderer_api.renderer_create_buffer = software_renderer::renderer_create_buffer;
            //renderer_api.renderer_destroy_buffer = software_renderer::renderer_destroy_buffer;
            //renderer_api.renderer_create_texture = software_renderer::renderer_create_texture;
            //renderer_api.renderer_destroy_texture = software_renderer::renderer_destroy_texture;
            //renderer_api.renderer_draw = software_renderer::renderer_draw;
        } break;
        
        case RENDERER_TYPE_VULKAN:
        {
            api.renderer_startup = vulkan_renderer::renderer_startup;
            //renderer_api.renderer_shutdown = vulkan_renderer::renderer_shutdown;
            //renderer_api.renderer_begin_frame = vulkan_renderer::renderer_begin_frame;
            //renderer_api.renderer_end_frame = vulkan_renderer::renderer_end_frame;
            //renderer_api.renderer_create_buffer = vulkan_renderer::renderer_create_buffer;
            //renderer_api.renderer_destroy_buffer = vulkan_renderer::renderer_destroy_buffer;
            //renderer_api.renderer_create_texture = vulkan_renderer::renderer_create_texture;
            //renderer_api.renderer_destroy_texture = vulkan_renderer::renderer_destroy_texture;
            //renderer_api.renderer_draw = vulkan_renderer::renderer_draw;
        } break;
    }

    return api.renderer_startup(renderer_config);
}

b8 renderer_shutdown()
{
    return api.renderer_shutdown();
}

void renderer_begin_frame()
{
    api.renderer_begin_frame();
}

void renderer_end_frame()
{
    api.renderer_end_frame();
}

buffer_handle renderer_create_buffer(s32 size, void* data)
{
    return api.renderer_create_buffer(size, data);
}

void renderer_destroy_buffer(buffer_handle buffer_handle)
{
    api.renderer_destroy_texture(buffer_handle);
}

texture_handle renderer_create_texture(s32 width, s32 height, void* data)
{
    return api.renderer_create_texture(width, height, data);
}

void renderer_destroy_texture(texture_handle texture_handle)
{
    api.renderer_destroy_texture(texture_handle);
}

void renderer_draw(void* vertex_array, void* material)
{
    api.renderer_draw(vertex_array, material);
}



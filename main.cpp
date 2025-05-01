#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <unistd.h>

#include "warpunk.core/platform/platform.h"
#include "warpunk.core/input_system/input_system.h"
#include "warpunk.core/renderer/renderer_backend.h"

//////////////////////////////////////////////////////////////////////

int main() 
{
    // Platform 
    if (!platform_startup())
    {
        fprintf(stderr, "Failed to startup platform.\n");
        return -1;
    }

    // Input system
    if (!input_system_startup())
    {
        fprintf(stderr, "Failed to startup input system.\n");
    }
    platform_register_keyboard_event(input_system_process_key);
    platform_register_mouse_button_event(input_system_process_mouse_button);
    platform_register_mouse_move_event(input_system_process_mouse_move);
    platform_register_mouse_wheel_event(input_system_process_mouse_wheel);

    // Renderer
    renderer_config_s renderer_config = {};
    renderer_config.type = RENDERER_TYPE_SOFTWARE;
    renderer_config.width = 1920 / 2;
    renderer_config.aspect_ratio = 16.0 / 9.0;
    renderer_config.flags = RENDERER_CONFIG_FLAG_VSYNC_ENABLED_BIT; 
    if (!renderer_startup(renderer_config))
    {
       fprintf(stderr, "Failed to startup renderer.\n"); 
    }

    renderer_config.type = RENDERER_TYPE_VULKAN;
    if (!renderer_startup(renderer_config))
    {
        fprintf(stderr, "Failed to startup renderer.\n");
    }

    // Game 
    library_context_s library_context = {};
    if (!platform_load_library("./bin/debug/magicians_misfits.so", &library_context))
    {
        fprintf(stderr, "shutdown warpunk.\n");
        return -1;
    }
    
    function_description_s game_startup_desc = {};
    game_startup_desc.name = "game_startup";
    if (!platform_get_function(&library_context, &game_startup_desc))
    {
        fprintf(stderr, "shutdown warpunk.\n");
        return -1; 
    }
    game_startup_t game_startup = (game_startup_t)game_startup_desc.function;
    game_startup();

    function_description_s game_tick_desc = {};
    game_tick_desc.name = "game_tick";
    if (!platform_get_function(&library_context, &game_tick_desc))
    {
        fprintf(stderr, "shutdown warpunk.\n");
        return -1;
    }
    game_tick_t game_tick = (game_tick_t)game_tick_desc.function;
    
    // Main lopp
    while (true)

    {
        platform_process_input();
        renderer_begin_frame();

        game_tick(0);
       
        input_system_update();
        usleep(16667);
    }
    
    return 0;
}

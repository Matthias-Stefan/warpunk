#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <unistd.h>

#include "warpunk.core/platform.h"
#include "warpunk.core/input_system.h"

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
    // Game 
    library_context_t library_context = {};
    if (!platform_load_library("./bin/debug/magicians_misfits.so", &library_context))
    {
        fprintf(stderr, "shutdown warpunk.\n");
        return -1;
    }
    
    function_description_t game_startup_desc = {};
    game_startup_desc.name = "game_startup";
    if (!platform_get_function(&library_context, &game_startup_desc))
    {
        fprintf(stderr, "shutdown warpunk.\n");
        return -1; 
    }
    game_startup_t game_startup = (game_startup_t)game_startup_desc.function;
    game_startup();

    function_description_t game_tick_desc = {};
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

        game_tick(0);
        
        input_system_update();
        usleep(16667);
    }
    
    return 0;
}


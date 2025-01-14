#include "../warpunk.core/platform.h"
#include "../warpunk.core/input_system.h"
#include <cstdio>

void game_startup()
{
}

void game_tick(double dt)
{
#if false 
    b8 is_key_a_up = input_system_is_key_up(KEY_A);
    b8 is_key_a_down = input_system_is_key_down(KEY_A);
    
    b8 was_key_a_up = input_system_was_key_up(KEY_A);
    b8 was_key_a_down = input_system_was_key_down(KEY_A);

    fprintf(stderr, "%s (is_up, is_down, was_up, was_down): %i, %i, %i, %i\n", input_system_keycode_str(KEY_A), is_key_a_up, is_key_a_down, was_key_a_up, was_key_a_down);
#endif

    platform_window_info_t window_info = {};
    if (platform_get_window_info(&window_info))
    {
        if (window_info.platform_window_mode == FULLSCREEN)
        {
            fprintf(stderr, "FULLSCREEN\n");
        }
        else if (window_info.platform_window_mode == WINDOWED)
        {
            fprintf(stderr, "WINDOWED\n");
        }
    }

    if (platform_is_mouse_inside_window())
    {
        fprintf(stderr, "Inside\n");
    }

     [[maybe_unused]] bool _;
    if (input_system_is_key_down(KEY_8) && !input_system_was_key_down(KEY_8))
    {
        _ = platform_set_window_mode(FULLSCREEN);
    }
    if (input_system_is_key_down(KEY_9) && !input_system_was_key_down(KEY_9))
    {
        _ = platform_set_window_mode(WINDOWED);
    }
}


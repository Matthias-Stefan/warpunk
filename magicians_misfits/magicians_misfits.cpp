#include <warpunk.core/platform//platform.h>
#include <warpunk.core/input_system/input_system.h>
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
    
    s16 x_abs, y_abs, x_rel, y_rel;
    input_system_get_mouse_position(&x_abs, &y_abs, &x_rel, &y_rel);
    fprintf(stderr, "mouse (abs, rel): %i, %i, %i, %i\n", x_abs, y_abs, x_rel, y_rel);

    for (int i = 0; i < MOUSE_BUTTON_COUNT; ++i)
    {
        if (input_system_is_mouse_button_pressed((mouse_button_t)i))
        {
            fprintf(stderr, "%i pressed\n", i);
        }
    }

    s32 delta_abs, delta_rel;
    if (input_system_get_mouse_wheel_delta(&delta_abs, &delta_rel))
    {
        fprintf(stderr, "mouse_delta: %i, %i\n", delta_abs, delta_rel);
    }

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


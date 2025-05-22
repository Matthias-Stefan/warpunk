#pragma once

#include "warpunk.core/src/defines.h"
#include "warpunk.core/src/input_system/input_types.h"
#include "warpunk.core/src/platform/platform.h"

typedef struct input_system_config
{
    s32 drag_threshold;
} input_system_config;

/** */
no_mangle warpunk_api b8 input_system_startup();

/** */
no_mangle warpunk_api void input_system_shutdown();

/** */
no_mangle warpunk_api void input_system_update();

/** */
no_mangle warpunk_api void input_system_configure(input_system_config input_system_config);

// KEYBOARD

/** */
no_mangle warpunk_api void input_system_process_key(keycode keycode, b8 pressed);

/** */
no_mangle warpunk_api void input_system_key_repeats_enable(b8 enable);

/** */
no_mangle warpunk_api b8 input_system_is_key_down(keycode keycode);

/** */
no_mangle warpunk_api b8 input_system_is_key_up(keycode keycode);

/** */
no_mangle warpunk_api b8 input_system_was_key_down(keycode keycode);

/** */
no_mangle warpunk_api b8 input_system_was_key_up(keycode keycode);

// MOUSE

/** */
no_mangle warpunk_api void input_system_process_mouse_button(mouse_button mouse_button, b8 pressed);

/** */
no_mangle warpunk_api void input_system_process_mouse_move(s16 x, s16 y);

/** */
no_mangle warpunk_api void input_system_process_mouse_wheel(s32 delta);

/** */
no_mangle warpunk_api b8 input_system_get_mouse_position(s16* x_abs, s16* y_abs, s16* x_rel, s16* y_rel);

/** */
no_mangle warpunk_api b8 input_system_is_mouse_button_pressed(mouse_button mouse_button);

/** */
no_mangle warpunk_api b8 input_system_was_mouse_button_pressed(mouse_button mouse_button);

/** */
no_mangle warpunk_api b8 input_system_get_mouse_wheel_delta(s32* delta_abs, s32* delta_rel);

/** */ 
no_mangle warpunk_api b8 input_system_is_mouse_inside_window();

/** */
no_mangle warpunk_api b8 input_system_mouse_is_dragging();

// MISC
//
/** */
no_mangle warpunk_api const char* input_system_keycode_str(keycode keycode);


#pragma once

#include "warpunk.core/defines.h"
#include "warpunk.core/input_system/input_types.h"
#include "warpunk.core/platform/platform.h"

typedef struct _input_system_config_t
{
    s32 drag_threshold;
} input_system_config_t;

/** */
b8 input_system_startup();

/** */
void input_system_shutdown();

/** */
void input_system_update();

/** */
void input_system_configure(input_system_config_t input_system_config);

// KEYBOARD

/** */
void input_system_process_key(keycode_t keycode, b8 pressed);

/** */
warpunk_api no_mangle void input_system_key_repeats_enable(b8 enable);

/** */
warpunk_api no_mangle b8 input_system_is_key_down(keycode_t keycode);

/** */
warpunk_api no_mangle b8 input_system_is_key_up(keycode_t keycode);

/** */
warpunk_api no_mangle b8 input_system_was_key_down(keycode_t keycode);

/** */
warpunk_api no_mangle b8 input_system_was_key_up(keycode_t keycode);

// MOUSE

/** */
void input_system_process_mouse_button(mouse_button_t mouse_button, b8 pressed);

/** */
void input_system_process_mouse_move(s16 x, s16 y);

/** */
void input_system_process_mouse_wheel(s32 delta);

/** */
warpunk_api no_mangle b8 input_system_get_mouse_position(s16* x_abs, s16* y_abs, s16* x_rel, s16* y_rel);

/** */
warpunk_api no_mangle b8 input_system_is_mouse_button_pressed(mouse_button_t mouse_button);

/** */
warpunk_api no_mangle b8 input_system_was_mouse_button_pressed(mouse_button_t mouse_button);

/** */
warpunk_api no_mangle b8 input_system_get_mouse_wheel_delta(s32* delta_abs, s32* delta_rel);

/** */ 
warpunk_api no_mangle b8 input_system_is_mouse_inside_window();

/** */
warpunk_api no_mangle b8 input_system_mouse_is_dragging();

// MISC
//
/** */
warpunk_api no_mangle const char* input_system_keycode_str(keycode_t keycode);


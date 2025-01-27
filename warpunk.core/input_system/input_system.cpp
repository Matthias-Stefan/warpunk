#include "warpunk.core/input_system/input_system.h"

#include "warpunk.core/defines.h"
#include "warpunk.core/input_system/input_types.h"
#include "warpunk.core/platform/platform.h"

//////////////////////////////////////////////////////////////////////

typedef struct _keyboard_t
{
    b8 allow_key_repeats;
    
    b8 current_key_state[KEYCODE_COUNT];
    b8 previous_key_state[KEYCODE_COUNT];
} keyboard_t;

typedef enum _mouse_time_state_t
{
    MOUSE_TIME_STATE_CURRENT,
    MOUSE_TIME_STATE_PREVIOUS,

    MOUSE_TIME_STATE_COUNT
} mouse_time_state_t;

typedef struct _mouse_t
{
    b8 inside_window;
    
    b8 current_key_state[MOUSE_BUTTON_COUNT];
    b8 previous_key_state[MOUSE_BUTTON_COUNT];
    s16 x[MOUSE_TIME_STATE_COUNT];
    s16 y[MOUSE_TIME_STATE_COUNT];
    s32 delta[MOUSE_TIME_STATE_COUNT];
} mouse_t;

typedef struct _input_state_t
{
    keyboard_t keyboard;
    mouse_t mouse;
    s32 drag_threshold = 5;
} input_state_t;

//////////////////////////////////////////////////////////////////////

// TODO: ptr and dynamic memory alloc!
static input_state_t input_state; 

//////////////////////////////////////////////////////////////////////

b8 input_system_startup()
{
    // KEYBOARD
    keyboard_t* keyboard = &input_state.keyboard;
    for (u32 key_index = 0; key_index < KEYCODE_COUNT; ++key_index)
    {
        keyboard->current_key_state[key_index] = false;
        keyboard->previous_key_state[key_index] = false;
    }
    keyboard->allow_key_repeats = false;

    // MOUSE
    mouse_t* mouse = &input_state.mouse;
    for (u32 mouse_button_index = 0; mouse_button_index < MOUSE_BUTTON_COUNT; ++mouse_button_index)
    {
        mouse->current_key_state[mouse_button_index] = false;
        mouse->previous_key_state[mouse_button_index] = false;
    }
    for (u32 mouse_time_state_index = 0; mouse_time_state_index < MOUSE_TIME_STATE_COUNT; ++mouse_time_state_index)
    {
        mouse->x[mouse_time_state_index] = 0;
        mouse->y[mouse_time_state_index] = 0;
        mouse->delta[mouse_time_state_index] = 0;
    }
    mouse->inside_window = false;

    return true;
}

void input_system_shutdown()
{
    
}

void input_system_update()
{
    // KEYBOARD
    keyboard_t* keyboard = &input_state.keyboard; 
    for (u32 key_index = 0; key_index < KEYCODE_COUNT; ++key_index)
    {
        keyboard->previous_key_state[key_index] = input_state.keyboard.current_key_state[key_index];
    }

    // MOUSE
    mouse_t* mouse = &input_state.mouse;
    for (u32 mouse_button_index = 0; mouse_button_index < MOUSE_BUTTON_COUNT; ++mouse_button_index)
    {
        mouse->previous_key_state[mouse_button_index] = mouse->current_key_state[mouse_button_index];
    }
    mouse->x[MOUSE_TIME_STATE_PREVIOUS] = mouse->x[MOUSE_TIME_STATE_CURRENT];
    mouse->y[MOUSE_TIME_STATE_PREVIOUS] = mouse->y[MOUSE_TIME_STATE_CURRENT];
    mouse->delta[MOUSE_TIME_STATE_PREVIOUS] = mouse->delta[MOUSE_TIME_STATE_CURRENT];
}

void input_system_configure(input_system_config_t input_system_config)
{
   input_state.drag_threshold = input_system_config.drag_threshold; 
}

//////////////////////////////////////////////////////////////////////

// KEYBOARD

void input_system_process_key(keycode_t keycode, b8 pressed)
{
    input_state.keyboard.current_key_state[keycode] = pressed;
}

void input_system_key_repeats_enable(b8 enable)
{
    input_state.keyboard.allow_key_repeats = enable;
}

b8 input_system_is_key_down(keycode_t keycode)
{
    return input_state.keyboard.current_key_state[keycode];
}

b8 input_system_is_key_up(keycode_t keycode)
{
    return !input_state.keyboard.current_key_state[keycode];
}

b8 input_system_was_key_down(keycode_t keycode)
{
    return input_state.keyboard.previous_key_state[keycode];
}

b8 input_system_was_key_up(keycode_t keycode)
{
    return !input_state.keyboard.previous_key_state[keycode];
}

//////////////////////////////////////////////////////////////////////

// MOUSE

void input_system_process_mouse_button(mouse_button_t mouse_button, b8 pressed)
{
    input_state.mouse.current_key_state[mouse_button] = pressed;
}

void input_system_process_mouse_move(s16 x, s16 y)
{
    input_state.mouse.x[MOUSE_TIME_STATE_CURRENT] = x;
    input_state.mouse.y[MOUSE_TIME_STATE_CURRENT] = y;
}

void input_system_process_mouse_wheel(s32 delta)
{
    input_state.mouse.delta[MOUSE_TIME_STATE_CURRENT] += delta;
}

b8 input_system_get_mouse_position(s16* x_abs, s16* y_abs, s16* x_rel, s16* y_rel)
{
    if (!input_system_is_mouse_inside_window())
    {
        return false;
    }

    mouse_t* mouse = &input_state.mouse;
    if (x_abs)
    {
        *x_abs = mouse->x[MOUSE_TIME_STATE_CURRENT];
    }
    if (y_abs)
    {
        *y_abs = mouse->y[MOUSE_TIME_STATE_CURRENT];
    }
    if (x_rel)
    {
        *x_rel = mouse->x[MOUSE_TIME_STATE_CURRENT] - mouse->x[MOUSE_TIME_STATE_PREVIOUS];
    }
    if (y_rel)
    {
        *y_rel = mouse->y[MOUSE_TIME_STATE_CURRENT] - mouse->y[MOUSE_TIME_STATE_PREVIOUS];
    }

    return true;
}

b8 input_system_is_mouse_button_pressed(mouse_button_t mouse_button)
{
    return input_state.mouse.current_key_state[mouse_button];
}

b8 input_system_was_mouse_button_pressed(mouse_button_t mouse_button)
{
    return input_state.mouse.previous_key_state[mouse_button];
}

b8 input_system_get_mouse_wheel_delta(s32 *delta_abs, s32 *delta_rel)
{
    if (!input_system_is_mouse_inside_window())
    {
        return false;
    }
    
    if (delta_abs)
    {
        *delta_abs = input_state.mouse.delta[MOUSE_TIME_STATE_CURRENT];
    }
    if (delta_rel)
    {
        *delta_rel = input_state.mouse.delta[MOUSE_TIME_STATE_CURRENT] - input_state.mouse.delta[MOUSE_TIME_STATE_PREVIOUS];
    }
    
    return true;
}

b8 input_system_is_mouse_inside_window()
{
    return platform_is_mouse_inside_window();
}

b8 input_system_mouse_is_dragging()
{
    return true;
}

//////////////////////////////////////////////////////////////////////

// MISC

const char* input_system_keycode_str(keycode_t keycode)
{
    switch (keycode)
    {
        case KEY_ESCAPE:
            return "ESC";
        case KEY_F1:
            return "F1";
        case KEY_F2:
            return "F2";
        case KEY_F3:
            return "F3";
        case KEY_F4:
            return "F4";
        case KEY_F5:
            return "F5";
        case KEY_F6:
            return "F6";
        case KEY_F7:
            return "F7";
        case KEY_F8:
            return "F8";
        case KEY_F9:
            return "F9";
        case KEY_F10:
            return "F10";
        case KEY_F11:
            return "F11";
        case KEY_F12:
            return "F12";
        case KEY_INSERT:
            return "INSERT";
        case KEY_DELETE:
            return "DEL";
        case KEY_HOME:
            return "HOME";
        case KEY_END:
            return "END";
        case KEY_PAGE_UP:
            return "PAGE UP";
        case KEY_PAGE_DOWN:
            return "PAGE DOWN";
        case KEY_CIRCUMFLEX:
            return "^";
        case KEY_0:
            return "0";
        case KEY_1:
            return "1";
        case KEY_2:
            return "2";
        case KEY_3:
            return "3";
        case KEY_4:
            return "4";
        case KEY_5:
            return "5";
        case KEY_6:
            return "6";
        case KEY_7:
            return "7";
        case KEY_8:
            return "8";
        case KEY_9:
            return "9";
        case KEY_SSHARP:
            return "ß";
        case KEY_ACUTE:
            return "";
        case KEY_TAB:
            return "TAB";
        case KEY_CAPS_LOCK:
            return "CAPS";
        case KEY_SHIFT_L:
            return "SHIFT L";
        case KEY_LESS:
            return "<";
        case KEY_CONTROL_L:
            return "CTRL L";
        case KEY_ALT_L:
            return "ALT L";
        case KEY_SPACE:
            return "SPACE";
        case KEY_ISO_LEVEL3_SHIFT:
            return "";
        case KEY_MENU:
            return "MENU";
        case KEY_CONTROL_R:
            return "CTRL R";
        case KEY_SHIFT_R:
            return "SHIFT R";
        case KEY_RETURN:
            return "RET";
        case KEY_BACKSPACE:
            return "BACKSPACE";
        case KEY_COMMA:
            return ",";
        case KEY_PERIOD:
            return "";
        case KEY_MINUS:
            return "-";
        case KEY_LEFT:
            return "LEFT";
        case KEY_UP:
            return "UP";
        case KEY_DOWN:
            return "DOWN";
        case KEY_RIGHT:
            return "RIGHT";
        case KEY_NUM_LOCK:
            return "NUM LOCK";
        case KEY_KP_DIVIDE:
            return "/";
        case KEY_KP_MULTIPLY:
            return "*";
        case KEY_KP_SUBTRACT:
            return "-";
        case KEY_KP_ADD:
            return "+";
        case KEY_KP_1:
            return "1";
        case KEY_KP_2:
            return "2";
        case KEY_KP_3:
            return "3";
        case KEY_KP_4:
            return "4";
        case KEY_KP_5:
            return "5";
        case KEY_KP_6:
            return "6";
        case KEY_KP_7:
            return "7";
        case KEY_KP_8:
            return "8";
        case KEY_KP_9:
            return "9";
        case KEY_KP_DELETE:
            return ",";
        case KEY_KP_ENTER:
            return "ENTER";
        case KEY_A:
            return "A";
        case KEY_B:
            return "B";
        case KEY_C:
            return "C";
        case KEY_D:
            return "D";
        case KEY_E:
            return "E";
        case KEY_F:
            return "F";
        case KEY_G:
            return "G";
        case KEY_H:
            return "H";
        case KEY_I:
            return "I";
        case KEY_J:
            return "J";
        case KEY_K:
            return "K";
        case KEY_L:
            return "L";
        case KEY_M:
            return "M";
        case KEY_N:
            return "N";
        case KEY_O:
            return "O";
        case KEY_P:
            return "P";
        case KEY_Q:
            return "Q";
        case KEY_R:
            return "R";
        case KEY_S:
            return "S";
        case KEY_T:
            return "T";
        case KEY_U:
            return "U";
        case KEY_V:
            return "V";
        case KEY_W:
            return "W";
        case KEY_X:
            return "X";
        case KEY_Y:
            return "Y";
        case KEY_Z:
            return "Z";
        default:
            return "";
    }
}






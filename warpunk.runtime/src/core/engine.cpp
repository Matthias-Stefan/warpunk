#include "warpunk.runtime/src/core/engine.h"

#include <warpunk.core/src/input_system/input_system.h>
#include <warpunk.core/src/platform/platform.h>
#include <warpunk.core/src/renderer/renderer_backend.h>
#include <warpunk.core/src/time/runtime_clock.h>

typedef struct engine_state
{
    application* app;
    b8 is_running;
    b8 is_suspended;

    runtime_clock clock;
    f64 last_time;
} engine_state;

// TODO: heap alloc
struct engine_state state = {};

b8 engine_create(struct application* app)
{
    state.is_running = true;

    // Platform system
    {
        // TODO: config
        if (!platform_startup())
        {
            WERROR("Failed to initialize platform system.");
            return false;
        }
    }

    // Input system
    {
        if (!input_system_startup())
        {
            WERROR("Failed to initialize input system.");
            return false;
        }

        platform_register_keyboard_event(input_system_process_key);
        platform_register_mouse_button_event(input_system_process_mouse_button);
        platform_register_mouse_move_event(input_system_process_mouse_move);
        platform_register_mouse_wheel_event(input_system_process_mouse_wheel);
    }

    // Renderer system
    {
        // TODO: Later from the config
        renderer_config config = {};
        config.type = RENDERER_TYPE_VULKAN;
        config.application_name = "Magicians Misfits";
        config.width = 1920 / 2;
        config.aspect_ratio = 16.0 / 9.0;
        config.flags = RENDERER_CONFIG_FLAG_VSYNC_ENABLED_BIT;
        if (!renderer_startup(config))
        {
            WERROR("Failed to initialize renderer system.");
            return false;
        }
    }

    return true;
}

b8 engine_run(struct application* app)
{
    f64 target_frame_seconds = 1.0f / 60;
    f64 frame_elapsed_time = 0;

    while (state.is_running)
    {
        WINFO("Test");
        platform_process_input();

        if (!state.is_suspended)
        {
            runtime_clock_update(&state.clock);
            f64 current_time = state.clock.elapsed;
            f64 delta = current_time - state.last_time; (void)delta;
            f64 frame_start_time = platform_get_absolute_time(); (void)frame_start_time;

            // TODO: frame

            f64 frame_end_time = platform_get_absolute_time();
            f64 frame_elapsed_time = frame_end_time - frame_start_time;
            f64 remaining_seconds = target_frame_seconds - frame_elapsed_time;

            state.last_time = current_time;
            WINFO("last time: %e", state.last_time);
        }
        else
        {
            WDEBUG("suspended...");
        }
    }

    platform_shutdown();
    return true;
}

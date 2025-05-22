#include "warpunk.runtime/src/core/engine.h"

#include <warpunk.core/src/time/runtime_clock.h>
#include <warpunk.core/src/platform/platform.h>

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

    return true;
}

b8 engine_run(struct application* app)
{
    while (state.is_running)
    {
        WINFO("Test");
        platform_process_input();

        if (!state.is_suspended)
        {
            runtime_clock_update(&state.clock);
            f64 current_time = state.clock.elapsed;
            f64 delta = current_time - state.last_time;
            // HACK:
            (void)delta;
            f64 frame_start_time = platform_get_absolute_time();
            // HACK:
            (void)frame_start_time;

            state.last_time = current_time;
        }
        else
        {
            WDEBUG("suspended...");
        }
    }

    platform_shutdown();
    return true;
}
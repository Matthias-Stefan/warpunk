#pragma once

#include <warpunk.core/src/defines.h>

typedef struct engine_system_states
{
    u64 platform_system_memory_requirement;
    struct platform_system_state* platform_system;

    u64 renderer_system_memory_requirement;
    struct renderer_system_state* renderer_system;
} engine_system_states;

/** */
no_mangle warpunk_api b8 engine_create(struct application* app);

/** */
no_mangle warpunk_api b8 engine_run(struct application* app);


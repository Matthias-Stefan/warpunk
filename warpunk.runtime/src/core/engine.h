#pragma once

#include <warpunk.core/src/defines.h>

typedef struct engine_system_states
{
    
} engine_system_states;

/** */
no_mangle warpunk_api b8 engine_create(struct application* app);

/** */
no_mangle warpunk_api b8 engine_run(struct application* app);


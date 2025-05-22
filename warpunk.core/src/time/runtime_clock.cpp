#include "warpunk.core/src/time/runtime_clock.h"
#include "warpunk.core/src/platform/platform.h"

void runtime_clock_update(runtime_clock* clock)
{
    if (clock->start_time != 0)
    {
        clock->elapsed = platform_get_absolute_time();
    }
}

void runtime_clock_start(runtime_clock* clock)
{
    clock->start_time = platform_get_absolute_time();
    clock->elapsed = 0;
}

void runtime_clock_stop(runtime_clock* clock)
{
    clock->start_time = 0;
}
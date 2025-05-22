#pragma once

#include "warpunk.core/src/defines.h"

/**
 * @brief High-resolution runtime clock for measuring elapsed time.
 */
typedef struct runtime_clock
{
    /**
     * @brief Timestamp when the clock was started (in seconds).
     */
    f64 start_time;

    /**
     * @brief Elapsed time since start (in seconds).
     */
    f64 elapsed;
} runtime_clock;

/**
 * @brief Updates the elapsed time of the clock.
 * @param clock Pointer to the runtime clock.
 */
no_mangle warpunk_api void runtime_clock_update(runtime_clock* clock);

/**
 * @brief Starts or restarts the clock.
 * @param clock Pointer to the runtime clock.
 */
no_mangle warpunk_api void runtime_clock_start(runtime_clock* clock);

/**
 * @brief Stops the clock and finalizes elapsed time.
 * @param clock Pointer to the runtime clock.
 */
no_mangle warpunk_api void runtime_clock_stop(runtime_clock* clock);

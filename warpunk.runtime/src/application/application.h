#pragma once

#include <warpunk.core/src/defines.h>

/** @brief Represents the core application interface with lifecycle callbacks. */
typedef struct application
{
    /** @brief Called before system initialization to perform early setup. */
    b8 (*boot)(struct application* app);

    /** @brief Updates application state once per frame. */
    b8 (*update)(struct application* app);

    /** @brief Prepares resources and state for rendering. */
    b8 (*prepare_frame)(struct application* app);

    /** @brief Executes all rendering operations for the current frame. */
    b8 (*render_frame)(struct application* app);

    /** @brief Cleans up resources and shuts down the application. */
    b8 (*shutdown)(struct application* app);
} application;


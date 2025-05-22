#pragma once

#include "warpunk.runtime/src/application/application.h"
#include "warpunk.runtime/src/core/engine.h"

#include <stdio.h>
#include <stdlib.h>
#include <warpunk.core/src/defines.h>
#include <warpunk.core/src/utils/logger.h>
#include <warpunk.core/src/platform/platform.h>
#include <warpunk.core/src/input_system/input_system.h>
#include <warpunk.core/src/renderer/renderer_backend.h>

extern b8 application_create(application* out_application);

extern b8 application_initialize(application* application);

/**
 * @brief The main entry point of the application.
 * @returns 0 on successful execution; nonzero on error.
 */
int main() 
{
    WINFO("WARPUNK ENGINE START: %d", 6);
    WSUCCESS("WARPUNK ENGINE START: %d", 5);
    WWARNING("WARPUNK ENGINE START: %d", 4);
    WERROR("WARPUNK ENGINE START: %d", 3);
    WFATAL("WARPUNK ENGINE START: %d", 2);
    WDEBUG("WARPUNK ENGINE START: %d", 1);
    WVERBOSE("WARPUNK ENGINE START: %d", 0);

    application app = {};

    // TODO: application configuration
    // <...code>

    if (!application_create(&app))
    {
        WFATAL("");
        return -1;
    }

    if (!app.update || !app.prepare_frame || !app.render_frame)
    {
        WFATAL("");
        return -2;
    }

    if (!engine_create(&app))
    {
        WFATAL("");
        return -3;
    }

    if (!application_initialize(&app))
    {
        WFATAL("");
        return -4;
    }

    if (!engine_run(&app))
    {
        WFATAL("");
        return -5;
    }

    return 0;
}

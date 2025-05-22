#include "warpunk.core/src/utils/logger.h"
#include "warpunk.core/src/platform/platform.h"

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

static PFN_console_write console_hook = 0;

void logger_console_write_hook_set(PFN_console_write hook)
{
    console_hook = hook;
}

void _log_output(log_level level, const char* message, ...)
{
    const char* level_strs[7] = {"[INFO]:    ", "[SUCCESS]: ", "[WARNING]: ", "[ERROR]:   ", "[FATAL]:   ", "[DEBUG]:   ", "[VERBOSE]: "};

    // TODO: use string in the future 
    char formatted_message[2048]; 
    
    __builtin_va_list arg_ptr;
    va_start(arg_ptr, message);
    vsnprintf(formatted_message, sizeof(formatted_message), message, arg_ptr);
    va_end(arg_ptr);
    
    char out_message[2048]; 
    snprintf(out_message, sizeof(out_message), "%s%s\n", level_strs[level], formatted_message);

    if (console_hook) 
    {
        console_hook(level, out_message);
    } 
    else 
    {
        platform_console_write(level, out_message);
    }

    if (level == LOG_LEVEL_FATAL) 
    {
        //warpunk_debug_break();
    }
}

void report_assertion_failure(const char* expression, const char* message, const char* file, s32 line) 
{
    _log_output(LOG_LEVEL_FATAL, "Assertion Failure: %s, message: '%s', [file:line]: %s:%d\n", expression, message, file, line);
}


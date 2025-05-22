#pragma once

#include "warpunk.core/src/defines.h"

/** @brief Enables logging for informational messages */
#define LOG_INFO_ENABLED 1
/** @brief Enables logging for success messages */
#define LOG_SUCCESS_ENABLED 1
/** @brief Enables logging for warning messages */
#define LOG_WARNING_ENABLED 1
/** @brief Enables logging for error messages */
#define LOG_ERROR_ENABLED 1
/** @brief Enables logging for fatal error messages */
#define LOG_FATAL_ENABLED 1
/** @brief Enables logging for debug messages */
#define LOG_DEBUG_ENABLED 1
/** @brief Enables logging for verbose messages (very detailed logs) */
#define LOG_VERBOSE_ENABLED 1

/** 
 * @brief Enumeration of log levels used to categorize log messages by severity or purpose.
 */
typedef enum log_level
{
    LOG_LEVEL_INFO    = 0,
    LOG_LEVEL_SUCCESS = 1,
    LOG_LEVEL_WARNING = 2,
    LOG_LEVEL_ERROR   = 3,
    LOG_LEVEL_FATAL   = 4,
    LOG_LEVEL_DEBUG   = 5,
    LOG_LEVEL_VERBOSE = 6,
} log_level;

/**
 * @brief Function pointer type for a custom console write function.
 * @param level The severity level of the message
 * @param message The message to be logged
 */
typedef void (*PFN_console_write)(log_level level, const char* message);

/**
 * @brief Registers a custom console write hook.
 * This allows redirecting logging output to a user-defined handler.
 * @param hook Pointer to a function that handles log output
 */
no_mangle warpunk_api void logger_console_write_hook_set(PFN_console_write hook);

/**
 * @brief Internal logging function that handles formatted log output.
 * Accepts a variable number of arguments similar to printf.
 * @param level The severity level of the log
 * @param message The format string
 */
no_mangle warpunk_api void _log_output(log_level level, const char* message, ...);


#if LOG_INFO_ENABLED == 1
/** @brief Logs an informational message if enabled */
#define WINFO(message, ...) _log_output(LOG_LEVEL_INFO, message, ##__VA_ARGS__);
#else
/** @brief Disabled: Logs no informational messages */
#define WINFO(message, ...)
#endif

#if LOG_SUCCESS_ENABLED == 1
/** @brief Logs a success message if enabled */
#define WSUCCESS(message, ...) _log_output(LOG_LEVEL_SUCCESS, message, ##__VA_ARGS__);
#else
/** @brief Disabled: Logs no success messages */
#define WSUCCESS(message, ...)
#endif

#if LOG_WARNING_ENABLED == 1
/** @brief Logs a warning message if enabled */
#define WWARNING(message, ...) _log_output(LOG_LEVEL_WARNING, message, ##__VA_ARGS__);
#else
/** @brief Disabled: Logs no warning messages */
#define WWARNING(message, ...)
#endif

#if LOG_ERROR_ENABLED == 1
/** @brief Logs an error message if enabled */
#define WERROR(message, ...) _log_output(LOG_LEVEL_ERROR, message, ##__VA_ARGS__);
#else
/** @brief Disabled: Logs no error messages */
#define WERROR(message, ...)
#endif

#if LOG_FATAL_ENABLED == 1
/** @brief Logs a fatal error message if enabled */
#define WFATAL(message, ...) _log_output(LOG_LEVEL_FATAL, message, ##__VA_ARGS__);
#else
/** @brief Disabled: Logs no fatal error messages */
#define WFATAL(message, ...)
#endif

#if LOG_DEBUG_ENABLED == 1
/** @brief Logs a debug message if enabled */
#define WDEBUG(message, ...) _log_output(LOG_LEVEL_DEBUG, message, ##__VA_ARGS__);
#else
/** @brief Disabled: Logs no debug messages */
#define WDEBUG(message, ...)
#endif

#if LOG_VERBOSE_ENABLED == 1
/** @brief Logs a verbose message if enabled */
#define WVERBOSE(message, ...) _log_output(LOG_LEVEL_VERBOSE, message, ##__VA_ARGS__);
#else
/** @brief Disabled: Logs no verbose messages */
#define WVERBOSE(message, ...)
#endif
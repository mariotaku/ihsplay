#include <stdio.h>
#include <stdarg.h>

#include "app_logging.h"

#include <SDL.h>
#include <PmLogLib.h>

static SDL_mutex *mutex = NULL;
static PmLogContext context;

void app_logging_init() {
    PmLogGetContext("ihsplay", &context);
}

void app_logging_deinit() {
    // No-op
}

void app_log_printf(app_log_level level, const char *tag, const char *fmt, ...) {
    char msg[1024];
    va_list arg;
    va_start(arg, fmt);
    vsnprintf(msg, 1023, fmt, arg);
    va_end(arg);
    msg[1023] = '\0';
    FILE *output = stdout;
    switch (level) {
        case APP_LOG_LEVEL_FATAL:
            output = stderr;
            PmLogCritical(context, tag, 0, "[%.03f] %s", ((float) SDL_GetTicks() / 1000.0f), msg);
            break;
        case APP_LOG_LEVEL_ERROR:
            output = stderr;
            PmLogError(context, tag, 0, "[%.03f] %s", ((float) SDL_GetTicks() / 1000.0f), msg);
            break;
        case APP_LOG_LEVEL_WARN:
            output = stderr;
            PmLogWarning(context, tag, 0, "[%.03f] %s", ((float) SDL_GetTicks() / 1000.0f), msg);
            break;
        case APP_LOG_LEVEL_INFO:
            PmLogInfo(context, tag, 0, "[%.03f] %s", ((float) SDL_GetTicks() / 1000.0f), msg);
            break;
        case APP_LOG_LEVEL_DEBUG:
        case APP_LOG_LEVEL_VERBOSE:
            PmLogDebug(context, tag, 0, "[%.03f] %s", ((float) SDL_GetTicks() / 1000.0f), msg);
            break;
    }
    fprintf(output, "[%.03f][%s] %s\n", ((float) SDL_GetTicks() / 1000.0f), tag, msg);
}

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

void app_log_printf(app_log_level level, const char *tag, const char *fmt, ...) {
    char msg[1024];
    va_list arg;
    va_start(arg, fmt);
    snprintf(msg, 1023, fmt, arg);
    msg[1023] = '\0';
    va_end(arg);
    PmLogInfo(context, tag, 0, "[%.03f] %s", ((float) SDL_GetTicks() / 1000.0f), msg);
    fprintf(stderr, "[%.03f][%s] %s\n", ((float) SDL_GetTicks() / 1000.0f), tag, msg);
}

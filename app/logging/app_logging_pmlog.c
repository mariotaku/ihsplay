#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include "app_logging.h"

#include <SDL.h>
#include <PmLogLib.h>

static bool log_header(int level, const char *name, const char *tag);

static SDL_mutex *mutex = NULL;
static PmLogContext context;

static size_t log_snprintf(char out[], size_t out_len, const char *fmt, va_list args) {
    vsnprintf(out, out_len, fmt, args);
    size_t msglen = strnlen(out, out_len);
    if (out[msglen - 1] == '\n') {
        msglen -= 1;
        out[msglen] = '\0';
    }
    return msglen;
}

void app_logging_init() {
    PmLogGetContext("ihsplay", &context);
}

void app_ihs_logf(IHS_LogLevel level, const char *tag, const char *fmt, ...) {
    char msg[1024];
    va_list arg;
    va_start(arg, fmt);
    log_snprintf(msg, 1024, fmt, arg);
    va_end(arg);
    PmLogInfo(context, tag, 0, "[%.03f] %s", (SDL_GetTicks() / 1000.0f), msg);
}

void app_ss4s_logf(SS4S_LogLevel level, const char *tag, const char *fmt, ...) {
    char msg[1024];
    va_list arg;
    va_start(arg, fmt);
    log_snprintf(msg, 1024, fmt, arg);
    va_end(arg);
    PmLogInfo(context, tag, 0, "[%.03f] %s", (SDL_GetTicks() / 1000.0f), msg);
}

void app_lv_log(const char *msg) {
    PmLogInfo(context, "LVGL", 0, "[%.03f] %s", (SDL_GetTicks() / 1000.0f), msg);
}

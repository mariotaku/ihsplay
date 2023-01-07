#include <stdio.h>
#include <stdarg.h>
#include <SDL_timer.h>
#include <SDL_mutex.h>

#include "app_logging.h"

static bool log_header(int level, const char *tag);

static SDL_mutex *lock = NULL;

void app_logging_init() {
    lock = SDL_CreateMutex();
}

void app_logging_deinit() {
    SDL_DestroyMutex(lock);
    lock = NULL;
}

void app_log_printf(app_log_level level, const char *tag, const char *fmt, ...) {
    if (lock == NULL) {
        return;
    }
    SDL_LockMutex(lock);
    if (!log_header(level, tag)) {
        SDL_UnlockMutex(lock);
        return;
    }
    va_list arg;
    va_start(arg, fmt);
    vfprintf(stderr, fmt, arg);
    va_end(arg);
    fprintf(stderr, "\x1b[0m\n");
    SDL_UnlockMutex(lock);
}

static bool log_header(int level, const char *tag) {
    switch (level) {
        case IHS_LogLevelInfo:
            fprintf(stderr, "[%.03f][%s]\x1b[36m ", (float) SDL_GetTicks() / 1000.0f, tag);
            break;
        case IHS_LogLevelWarn:
            fprintf(stderr, "[%.03f][%s]\x1b[33m ", (float) SDL_GetTicks() / 1000.0f, tag);
            break;
        case IHS_LogLevelError:
            fprintf(stderr, "[%.03f][%s]\x1b[31m ", (float) SDL_GetTicks() / 1000.0f, tag);
            break;
        case IHS_LogLevelFatal:
            fprintf(stderr, "[%.03f][%s]\x1b[41m ", (float) SDL_GetTicks() / 1000.0f, tag);
            break;
        case IHS_LogLevelVerbose:
            return false;
        default:
            fprintf(stderr, "[%.03f][%s] ", (float) SDL_GetTicks() / 1000.0f, tag);
            break;
    }
    return true;
}
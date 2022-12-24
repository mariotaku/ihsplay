#include <stdio.h>
#include <stdarg.h>

#include "app_logging.h"

static bool log_header(int level, const char *tag);

void app_logging_init() {
    // No-op
}

void app_log_printf(app_log_level level, const char *tag, const char *fmt, ...) {
    if (!log_header(level, tag)) {
        return;
    }
    va_list arg;
    va_start(arg, fmt);
    vfprintf(stderr, fmt, arg);
    va_end(arg);
    fprintf(stderr, "\x1b[0m\n");
}

static bool log_header(int level, const char *tag) {
    switch (level) {
        case IHS_LogLevelInfo:
            fprintf(stderr, "[%s]\x1b[36m ", tag);
            break;
        case IHS_LogLevelWarn:
            fprintf(stderr, "[%s]\x1b[33m ", tag);
            break;
        case IHS_LogLevelError:
            fprintf(stderr, "[%s]\x1b[31m ", tag);
            break;
        case IHS_LogLevelFatal:
            fprintf(stderr, "[%s]\x1b[41m ", tag);
            break;
        case IHS_LogLevelVerbose:
            return false;
        default:
            fprintf(stderr, "[%s] ", tag);
            break;
    }
    return true;
}
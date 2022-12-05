#include <stdio.h>
#include <stdarg.h>

#include "ihslib/common.h"

static bool log_header(IHS_LogLevel level, const char *tag);

void app_ihs_log(IHS_LogLevel level, const char *tag, const char *message) {
    if (!log_header(level, tag)) {
        return;
    }
    fprintf(stderr, "%s\x1b[0m\n", message);
}

void app_ihs_vlog(IHS_LogLevel level, const char *tag, const char *fmt, ...) {
    if (!log_header(level, tag)) {
        return;
    }
    va_list arg;
    va_start(arg, fmt);
    vfprintf(stderr, fmt, arg);
    va_end(arg);
    fprintf(stderr, "\x1b[0m\n");
}

static bool log_header(IHS_LogLevel level, const char *tag) {
    switch (level) {
        case IHS_LogLevelInfo:
            fprintf(stderr, "[IHS.%s]\x1b[36m ", tag);
            break;
        case IHS_LogLevelWarn:
            fprintf(stderr, "[IHS.%s]\x1b[33m ", tag);
            break;
        case IHS_LogLevelError:
            fprintf(stderr, "[IHS.%s]\x1b[31m ", tag);
            break;
        case IHS_LogLevelFatal:
            fprintf(stderr, "[IHS.%s]\x1b[41m ", tag);
            break;
        case IHS_LogLevelVerbose:
            return false;
        default:
            fprintf(stderr, "[IHS.%s] ", tag);
            break;
    }
    return true;
}
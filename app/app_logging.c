#include <stdio.h>
#include <stdarg.h>
#include <string.h>

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

void app_lv_log(const char *msg) {
    const char *start = strchr(msg, '\t') + 1;
    static const IHS_LogLevel level_value[] = {
            IHS_LogLevelVerbose, IHS_LogLevelInfo, IHS_LogLevelWarn, IHS_LogLevelError, IHS_LogLevelDebug,
    };
    static const char *level_name[] = {
            "Trace", "Info", "Warn", "Error", "User",
    };
    for (int i = 0; i < sizeof(level_value) / sizeof(IHS_LogLevel); i++) {
        if (strncmp(level_name[i], msg + 1, (start - msg - 3)) == 0) {
            app_ihs_log(level_value[i], "LVGL", start);
        }
    }
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
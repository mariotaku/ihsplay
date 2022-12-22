#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include "app_logging.h"

static bool log_header(int level, const char *name, const char *tag);

void app_logging_init() {
    // No-op
}

void app_ihs_logf(IHS_LogLevel level, const char *tag, const char *fmt, ...) {
    if (!log_header(level, "IHS", tag)) {
        return;
    }
    va_list arg;
    va_start(arg, fmt);
    vfprintf(stderr, fmt, arg);
    va_end(arg);
    fprintf(stderr, "\x1b[0m\n");
}

void app_ss4s_logf(SS4S_LogLevel level, const char *tag, const char *fmt, ...) {
    if (!log_header(level, "SS4S", tag)) {
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
            return;
        }
    }
}

static bool log_header(int level, const char *name, const char *tag) {
    switch (level) {
        case IHS_LogLevelInfo:
            fprintf(stderr, "[%s.%s]\x1b[36m ", name, tag);
            break;
        case IHS_LogLevelWarn:
            fprintf(stderr, "[%s.%s]\x1b[33m ", name, tag);
            break;
        case IHS_LogLevelError:
            fprintf(stderr, "[%s.%s]\x1b[31m ", name, tag);
            break;
        case IHS_LogLevelFatal:
            fprintf(stderr, "[%s.%s]\x1b[41m ", name, tag);
            break;
        case IHS_LogLevelVerbose:
            return false;
        default:
            fprintf(stderr, "[%s.%s] ", name, tag);
            break;
    }
    return true;
}
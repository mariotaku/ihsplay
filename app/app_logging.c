#include <stdio.h>

#include "ihslib/common.h"

void app_ihs_log(IHS_LogLevel level, const char *tag, const char *message) {
    switch (level) {
        case IHS_LogLevelInfo:
            fprintf(stderr, "[IHS.%s]\x1b[36m %s\x1b[0m\n", tag, message);
            break;
        case IHS_LogLevelWarn:
            fprintf(stderr, "[IHS.%s]\x1b[33m %s\x1b[0m\n", tag, message);
            break;
        case IHS_LogLevelError:
            fprintf(stderr, "[IHS.%s]\x1b[31m %s\x1b[0m\n", tag, message);
            break;
        case IHS_LogLevelFatal:
            fprintf(stderr, "[IHS.%s]\x1b[41m %s\x1b[0m\n", tag, message);
            break;
        default:
            fprintf(stderr, "[IHS.%s] %s\n", tag, message);
            break;
    }
}
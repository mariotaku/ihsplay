#include "app_logging.h"

#include <string.h>

void app_ihs_log(IHS_LogLevel level, const char *tag, const char *message) {
    const char *cur = message;
    do {
        const char *start = cur;
        cur = strchr(cur, '\n');
        if (cur != NULL) {
            int line_len = cur - start;
            if (line_len <= 0) {
                break;
            }
            app_ihs_logf(level, tag, "%.*s", line_len, start);
            cur = cur + 1;
        } else if (start[0] != '\0') {
            app_ihs_logf(level, tag, "%s", start);
        }
    } while (cur != NULL);
}
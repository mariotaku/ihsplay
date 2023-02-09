#include "app_logging.h"

#include <string.h>
#include <stdarg.h>
#include <stdio.h>

static void app_lv_log_line(const char *line, size_t len);

void app_log_hexdump(app_log_level level, const char *tag, const uint8_t *data, size_t len) {
    char line[80];
    static const char hex_table[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};
    for (int i = 0; i < len; i += 16) {
        int offset = 0;
        snprintf(line, 80, "%04x  ", i);
        offset += 6;
        for (int col = i, col_end = i + 16; col < col_end; ++col) {
            if (col >= len) {
                line[offset++] = ' ';
                line[offset++] = ' ';
            } else {
                line[offset++] = hex_table[(data[col] >> 4) & 0xF];
                line[offset++] = hex_table[(data[col]) & 0xF];
            }
            line[offset++] = ' ';
            if (col - i == 7) {
                line[offset++] = ' ';
            }
        }
        line[offset++] = ' ';
        line[offset++] = '|';
        for (int col = i, col_end = i + 16; col < col_end && col < len; ++col) {
            if (col >= len) {
                line[offset++] = ' ';
            } else {
                uint8_t ch = data[col];
                line[offset++] = ch >= ' ' && ch <= '~' ? ch : '.';
            }
        }
        line[offset++] = '|';
        line[offset] = '\0';
        app_log_printf(level, tag, "%s", line);
    }
}

void app_ihs_log(IHS_LogLevel level, const char *tag, const char *message) {
    char app_tag[32] = "IHS.";
    strncpy(app_tag + 4, tag, 28);
    app_log_printf((app_log_level) level, app_tag, "%s", message);
}

void app_ss4s_logf(SS4S_LogLevel level, const char *tag, const char *fmt, ...) {
    char app_tag[32] = "SS4S.";
    strncpy(app_tag + 5, tag, 27);
    va_list arg;
    va_start(arg, fmt);
    char msg[1024];
    vsnprintf(msg, 1024, fmt, arg);
    va_end(arg);
    app_log_printf((app_log_level) level, app_tag, "%s", msg);
}

void app_lv_log(const char *message) {
    const char *cur = message;
    do {
        const char *start = cur;
        cur = strchr(cur, '\n');
        if (cur != NULL) {
            int line_len = cur - start;
            if (line_len <= 0) {
                break;
            }
            app_lv_log_line(start, line_len);
            cur = cur + 1;
        } else if (start[0] != '\0') {
            app_lv_log_line(start, strlen(start));
        }
    } while (cur != NULL);

}

void app_sdl_log(void *userdata, int category, SDL_LogPriority priority, const char *message) {
    (void) userdata;
    static const app_log_level priority_name[SDL_NUM_LOG_PRIORITIES] = {
            APP_LOG_LEVEL_VERBOSE,
            APP_LOG_LEVEL_DEBUG,
            APP_LOG_LEVEL_INFO,
            APP_LOG_LEVEL_WARN,
            APP_LOG_LEVEL_ERROR,
            APP_LOG_LEVEL_FATAL
    };
    static const char *category_name[SDL_LOG_CATEGORY_TEST + 1] = {
            "SDL.APPLICATION",
            "SDL.ERROR",
            "SDL.ASSERT",
            "SDL.SYSTEM",
            "SDL.AUDIO",
            "SDL.VIDEO",
            "SDL.RENDER",
            "SDL.INPUT",
            "SDL.TEST",
    };
    const char *tag = category > SDL_LOG_CATEGORY_TEST ? "SDL" : category_name[category];
    app_log_printf(priority_name[priority - SDL_LOG_PRIORITY_VERBOSE], tag, "%s", message);
}

static void app_lv_log_line(const char *line, size_t len) {
    const char *start = memchr(line, '\t', len) + 1;
    static const app_log_level level_value[] = {
            APP_LOG_LEVEL_VERBOSE, APP_LOG_LEVEL_INFO, APP_LOG_LEVEL_WARN, APP_LOG_LEVEL_ERROR, APP_LOG_LEVEL_DEBUG,
    };
    static const char *level_name[] = {
            "Trace", "Info", "Warn", "Error", "User",
    };
    for (int i = 0; i < sizeof(level_value) / sizeof(IHS_LogLevel); i++) {
        if (strncmp(level_name[i], line + 1, (start - line - 3)) == 0) {
            app_log_printf(level_value[i], "LVGL", "%s", start);
            return;
        }
    }
}
#pragma once

#include <stddef.h>
#include "ihslib/common.h"
#include "ss4s/logging.h"

typedef enum app_log_level {
    APP_LOG_LEVEL_FATAL,
    APP_LOG_LEVEL_ERROR,
    APP_LOG_LEVEL_WARN,
    APP_LOG_LEVEL_INFO,
    APP_LOG_LEVEL_DEBUG,
    APP_LOG_LEVEL_VERBOSE,
} app_log_level;

void app_logging_init();

void app_log_printf(app_log_level level, const char *tag, const char *fmt, ...) __attribute__ ((format (printf, 3, 4)));

void app_log_hexdump(app_log_level level, const char *tag, const uint8_t *data, size_t len);

#define app_log_fatal(tag, ...) app_log_printf(APP_LOG_LEVEL_FATAL, (tag), __VA_ARGS__)

#define app_log_error(tag, ...) app_log_printf(APP_LOG_LEVEL_ERROR, (tag), __VA_ARGS__)

#define app_log_warn(tag, ...) app_log_printf(APP_LOG_LEVEL_WARN, (tag), __VA_ARGS__)

#define app_log_info(tag, ...) app_log_printf(APP_LOG_LEVEL_INFO, (tag), __VA_ARGS__)

#define app_log_debug(tag, ...) app_log_printf(APP_LOG_LEVEL_DEBUG, (tag), __VA_ARGS__)

#define app_log_verbose(tag, ...) app_log_printf(APP_LOG_LEVEL_VERBOSE, (tag), __VA_ARGS__)

void app_ihs_log(IHS_LogLevel level, const char *tag, const char *message);

void app_ss4s_logf(SS4S_LogLevel level, const char *tag, const char *fmt, ...) __attribute__ ((format (printf, 3, 4)));

void app_lv_log(const char *message);
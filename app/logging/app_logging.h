#pragma once

#include "ihslib/common.h"
#include "ss4s/logging.h"

void app_logging_init();

void app_ihs_log(IHS_LogLevel level, const char *tag, const char *message);

void app_ihs_logf(IHS_LogLevel level, const char *tag, const char *fmt, ...) __attribute__ ((format (printf, 3, 4)));

void app_ss4s_logf(SS4S_LogLevel level, const char *tag, const char *fmt, ...) __attribute__ ((format (printf, 3, 4)));

void app_lv_log(const char *msg);

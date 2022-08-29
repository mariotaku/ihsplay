#pragma once

#include <stdbool.h>

typedef struct app_settings_t {
    bool relmouse;
} app_settings_t;

void app_settings_initialize(app_settings_t *settings);
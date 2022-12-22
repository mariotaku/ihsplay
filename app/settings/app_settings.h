#pragma once

#include <stdbool.h>

typedef struct array_list_t array_list_t;

typedef struct app_settings_t {
    bool relmouse;
    char *audio_driver;
    char *video_driver;
    array_list_t *modules;
} app_settings_t;

void app_settings_init(app_settings_t *settings);

void app_settings_deinit(app_settings_t *settings);
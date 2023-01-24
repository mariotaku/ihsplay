#pragma once

#include <stdbool.h>
#include <stdint.h>

typedef struct array_list_t array_list_t;
typedef struct os_info_t os_info_t;

typedef struct app_settings_t {
    bool relmouse;
    /** The pointer references to modules */
    const char *audio_driver;
    /** The pointer references to modules */
    const char *video_driver;
    array_list_t *modules;
    uint64_t selected_client_id;
} app_settings_t;

void app_settings_init(app_settings_t *settings, const os_info_t *os_info);

void app_settings_deinit(app_settings_t *settings);
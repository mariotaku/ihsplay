#pragma once

#include <stdbool.h>
#include <lvgl.h>

#include "ihslib.h"

static const uint64_t deviceId = 11451419190810;

static const uint8_t secretKey[32] = {
        11, 45, 14, 19, 19, 8, 1, 0,
        11, 45, 14, 19, 19, 8, 1, 0,
        11, 45, 14, 19, 19, 8, 1, 0,
        11, 45, 14, 19, 19, 8, 1, 0,
};

static const char deviceName[] = "BABYLON STAGE34\0";

static const IHS_ClientConfig clientConfig = {deviceId, secretKey, deviceName};

typedef struct app_ui_t app_ui_t;

typedef struct app_t {
    bool running;
    app_ui_t *ui;
    IHS_Client *client;
} app_t;

app_t *app_create(lv_disp_t *disp);

void app_destroy(app_t *app);

void app_quit(app_t *app);

void app_discovery_broadcast(app_t *app);
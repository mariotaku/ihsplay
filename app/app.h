#pragma once

#include <stdbool.h>
#include <lvgl.h>
#include <SDL.h>

#include "ihslib.h"
#include "backend/hosts_manager.h"

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
    host_manager_t *hosts_manager;
} app_t;

typedef enum app_event_type_t {
    APP_EVENT_BEGIN = SDL_USEREVENT,
    APP_RUN_ON_MAIN,
    APP_EVENT_SIZE = (APP_RUN_ON_MAIN - APP_EVENT_BEGIN) + 1
} app_event_type_t;

app_t *app_create(lv_disp_t *disp);

void app_destroy(app_t *app);

void app_quit(app_t *app);

void app_run_on_main(app_t *app, void(*action)(app_t *, void *), void *data);

void *app_run_on_main_sync(app_t *app, void *(*action)(app_t *, void *), void *data);
#include <stdlib.h>
#include <stdio.h>
#include <SDL.h>

#include "app.h"
#include "ui/app_ui.h"
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

app_t *app_create(lv_disp_t *disp) {
    app_t *app = calloc(1, sizeof(app_t));
    app->running = true;
    app->client_config = clientConfig;
    app->hosts_manager = host_manager_create(app);
    app->ui = app_ui_create(app, disp);
    return app;
}

void app_destroy(app_t *app) {
    app_ui_destroy(app->ui);
    host_manager_destroy(app->hosts_manager);
    free(app);
}

void app_quit(app_t *app) {
    app->running = false;
}

void app_run_on_main(app_t *app, void(*action)(app_t *, void *), void *data) {
    SDL_Event event;
    event.user.type = APP_RUN_ON_MAIN;
    event.user.data1 = action;
    event.user.data2 = data;
    SDL_PushEvent(&event);
}

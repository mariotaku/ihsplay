#include <stdlib.h>
#include <assert.h>

#include "app.h"
#include "ui/app_ui.h"
#include "backend/host_manager.h"
#include "backend/stream_manager.h"
#include "backend/input_manager.h"

static const uint8_t secretKey[32] = {
        11, 45, 14, 19, 19, 8, 1, 0,
        11, 45, 14, 19, 19, 8, 1, 0,
        11, 45, 14, 19, 19, 8, 1, 0,
        11, 45, 14, 19, 19, 8, 1, 0,
};

static const IHS_ClientConfig clientConfig = {
        .deviceId = 11451419190810,
        .secretKey = secretKey,
        .deviceName = "BABYLON STAGE34"
};

app_t *app_create(app_settings_t *settings, void *disp) {
    assert(settings != NULL);
    assert(disp != NULL);
    app_t *app = calloc(1, sizeof(app_t));
    app->settings = settings;
    app->main_thread_id = SDL_ThreadID();
    app->running = true;
    app->client_config = clientConfig;
    app->input_manager = input_manager_create();
    app->host_manager = host_manager_create(app);
    app->stream_manager = stream_manager_create(app);
    app->ui = app_ui_create(app, (lv_disp_t *) disp);
    app_ui_created(app->ui);
    return app;
}

void app_destroy(app_t *app) {
    app_ui_destroy(app->ui);
    stream_manager_destroy(app->stream_manager);
    host_manager_destroy(app->host_manager);
    input_manager_destroy(app->input_manager);
    free(app);
}

void app_quit(app_t *app) {
    stream_manager_stop_active(app->stream_manager);
    app->running = false;
}

void app_assert_main_thread(app_t *app) {
    assert(app->main_thread_id == SDL_ThreadID());
}
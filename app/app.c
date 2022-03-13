#include <stdlib.h>
#include <stdio.h>
#include <SDL.h>

#include "app.h"
#include "ui/app_ui.h"
#include "backend/hosts_manager.h"

static void client_log(IHS_LogLevel level, const char *message);

app_t *app_create(lv_disp_t *disp) {
    app_t *app = calloc(1, sizeof(app_t));
    app->running = true;
    app->client = IHS_ClientCreate(&clientConfig);
    app->hosts_manager = host_manager_create(app);
    IHS_ClientSetLogFunction(app->client, client_log);
    IHS_ClientThreadedStart(app->client);
    app->ui = app_ui_create(app, disp);
    return app;
}

void app_destroy(app_t *app) {
    app_ui_destroy(app->ui);
    IHS_ClientDestroy(app->client);
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

static void client_log(IHS_LogLevel level, const char *message) {
    printf("[IHSClient] %s", message);
}

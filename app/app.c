#include <stdlib.h>
#include <stdio.h>
#include <SDL.h>

#include "app.h"
#include "ui/app_ui.h"
#include "backend/hosts_manager.h"

app_t *app_create(lv_disp_t *disp) {
    app_t *app = calloc(1, sizeof(app_t));
    app->running = true;
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

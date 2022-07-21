#pragma once

#include <stdbool.h>
#include <lvgl.h>
#include <SDL.h>

#include "ihslib.h"
#include "backend/hosts_manager.h"

typedef struct app_ui_t app_ui_t;

typedef struct app_t {
    bool running;
    app_ui_t *ui;
    IHS_ClientConfig client_config;
    host_manager_t *hosts_manager;
    IHS_Session *active_session;
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
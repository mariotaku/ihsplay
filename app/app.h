#pragma once

#include <stdbool.h>
#include <SDL.h>

#include "ihslib.h"
#include "ss4s.h"
#include "settings/app_settings.h"
#include "util/client_info.h"

typedef struct app_ui_t app_ui_t;
typedef struct stream_manager_t stream_manager_t;
typedef struct host_manager_t host_manager_t;
typedef struct input_manager_t input_manager_t;

typedef struct app_t {
    bool running;
    SDL_threadID main_thread_id;
    app_ui_t *ui;
    app_settings_t *settings;
    client_info_t client_info;
    host_manager_t *host_manager;
    stream_manager_t *stream_manager;
    input_manager_t *input_manager;
} app_t;

typedef enum app_event_type_t {
    APP_EVENT_BEGIN = SDL_USEREVENT,
    APP_RUN_ON_MAIN,
    APP_UI_EVENT_BEGIN,
    APP_UI_NAV_QUIT,
    APP_UI_NAV_BACK,
    APP_UI_REQUEST_OVERLAY,
    APP_UI_CLOSE_OVERLAY,
    APP_UI_EVENT_LAST,
    APP_EVENT_LAST,
} app_event_type_t;

typedef void(*app_run_action_fn)(app_t *, void *);

void app_preinit(int argc, char *argv[]);

app_t *app_create(app_settings_t *settings, void *disp);

void app_destroy(app_t *app);

void app_quit(app_t *app);

void app_post_event(app_t *app, app_event_type_t type, void *data1, void *data2);

void app_run_on_main(app_t *app, app_run_action_fn action, void *data);

void app_run_on_main_sync(app_t *app, app_run_action_fn action, void *data);

void app_sdl_input_event(app_t *app, const SDL_Event *event);

void app_assert_main_thread(app_t *app);
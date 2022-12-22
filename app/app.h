#pragma once

#include <stdbool.h>
#include <SDL.h>

#include "ihslib.h"
#include "settings/app_settings.h"

typedef struct app_ui_t app_ui_t;
typedef struct stream_manager_t stream_manager_t;
typedef struct host_manager_t host_manager_t;
typedef struct input_manager_t input_manager_t;

typedef struct app_t {
    bool running;
    SDL_threadID main_thread_id;
    app_ui_t *ui;
    app_settings_t *settings;
    IHS_ClientConfig client_config;
    host_manager_t *host_manager;
    stream_manager_t *stream_manager;
    input_manager_t *input_manager;
} app_t;

typedef enum app_event_type_t {
    APP_EVENT_BEGIN = SDL_USEREVENT,
    APP_RUN_ON_MAIN,
    APP_UI_EVENT_BEGIN,
    APP_UI_REQUEST_OVERLAY,
    APP_UI_CLOSE_OVERLAY,
    APP_UI_EVENT_LAST,
    APP_EVENT_LAST,
} app_event_type_t;

typedef void(*app_run_action_fn)(app_t *, void *);

app_t *app_create(app_settings_t*settings, void *disp);

void app_destroy(app_t *app);

void app_quit(app_t *app);

void app_post_event(app_t *app, app_event_type_t type, void *data1, void *data2);

void app_run_on_main(app_t *app, app_run_action_fn action, void *data);

void app_run_on_main_sync(app_t *app, app_run_action_fn action, void *data);

void app_sdl_gamepad_event(app_t *app, const SDL_Event *event);

void app_ihs_log(IHS_LogLevel level, const char *tag, const char *message);

void app_ihs_logf(IHS_LogLevel level, const char *tag, const char *fmt, ...) __attribute__ ((format (printf, 3, 4)));

void app_lv_log(const char *msg);

void app_assert_main_thread(app_t *app);
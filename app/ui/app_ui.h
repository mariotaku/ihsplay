#pragma once

#include <lvgl.h>
#include <SDL.h>

typedef struct app_t app_t;

typedef struct app_ui_t {
    app_t *app;
    lv_obj_t *root;
    lv_fragment_manager_t *fm;
    SDL_Window *window;
} app_ui_t;

typedef struct app_ui_fragment_args_t {
    app_t *app;
    void *data;
} app_ui_fragment_args_t;

app_ui_t *app_ui_create(app_t *app, lv_disp_t *disp);

void app_ui_destroy(app_ui_t *ui);

void app_ui_push_fragment(app_ui_t *ui, const lv_fragment_class_t *cls, void *args);

void app_ui_pop_fragment(app_ui_t *ui);
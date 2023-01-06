#pragma once

#include <lvgl.h>
#include <SDL.h>
#include "app.h"
#include "app_ui_font.h"

typedef struct app_t app_t;


typedef struct app_ui_t {
    app_t *app;
    lv_theme_t theme;
    lv_obj_t *root;
    lv_fragment_manager_t *fm;
    SDL_Window *window;
    app_ui_fontset_t font;
    app_ui_fontset_t iconfont;
    lv_group_t *group;
    lv_ll_t modal_groups;
    struct {
        lv_indev_t *mouse;
        lv_indev_t *keypad;
    } indev;
    struct {
        lv_style_t action_btn_label;
    } styles;
} app_ui_t;

typedef struct app_ui_fragment_args_t {
    app_t *app;
    void *data;
} app_ui_fragment_args_t;

typedef struct app_ui_event_data_t {
    void *data1;
    void *data2;
} app_ui_event_data_t;

app_ui_t *app_ui_create(app_t *app, lv_disp_t *disp);

void app_ui_created(app_ui_t *ui);

void app_ui_destroy(app_ui_t *ui);

void app_ui_resized(app_ui_t *ui, int width, int height);

void app_ui_set_ignore_keys(app_ui_t *ui, bool ignore);

void app_ui_push_fragment(app_ui_t *ui, const lv_fragment_class_t *cls, void *args);

void app_ui_pop_fragment(app_ui_t *ui);

/**
 * @warning DO NOT call this directly!
 * @return true if this event has been handled by someone
 */
bool app_ui_dispatch_event(app_ui_t *ui, app_event_type_t type, app_ui_event_data_t *data);

void app_ui_push_modal_group(app_ui_t *ui, lv_group_t *group);

void app_ui_remove_modal_group(app_ui_t *ui, lv_group_t *group);

lv_group_t *app_ui_get_input_group(app_ui_t *ui);

void app_ui_update_nav_back(app_ui_t *ui);

void app_ui_set_handle_nav_back(app_ui_t *ui, bool handle);
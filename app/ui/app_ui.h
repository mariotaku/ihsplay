#pragma once

#include <lvgl.h>
#include <SDL.h>

typedef struct app_t app_t;

typedef struct app_ui_fontset_t {
    struct {
        uint16_t small;
        uint16_t normal;
        uint16_t large;
    } size;
    lv_font_t *small;
    lv_font_t *normal;
    lv_font_t *large;
} app_ui_fontset_t;

typedef struct app_ui_t {
    app_t *app;
    lv_obj_t *root;
    lv_fragment_manager_t *fm;
    SDL_Window *window;
    app_ui_fontset_t iconfont;
    lv_group_t *group;
    struct {
        lv_indev_t *mouse;
        lv_indev_t *keypad;
    } indev;
} app_ui_t;

typedef struct app_ui_fragment_args_t {
    app_t *app;
    void *data;
} app_ui_fragment_args_t;

app_ui_t *app_ui_create(app_t *app, lv_disp_t *disp);

void app_ui_created(app_ui_t *ui);

void app_ui_destroy(app_ui_t *ui);

void app_ui_set_ignore_keys(app_ui_t *ui , bool ignore);

void app_ui_push_fragment(app_ui_t *ui, const lv_fragment_class_t *cls, void *args);

void app_ui_pop_fragment(app_ui_t *ui);

void app_ui_fontset_set_default_size(const app_ui_t *ui, app_ui_fontset_t *set);

void app_ui_fontset_init_mem(app_ui_fontset_t *set, const char *name, const void *mem, size_t size);

void app_ui_fontset_deinit(app_ui_fontset_t *set);

void app_ui_sdl_event(app_ui_t *ui, const SDL_Event *event);
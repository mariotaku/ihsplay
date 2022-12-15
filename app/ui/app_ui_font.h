#pragma once

#include <stdint.h>
#include "lvgl.h"

typedef struct app_t app_t;
typedef struct app_ui_t app_ui_t;

typedef struct app_ui_fontset_t {
    struct {
        uint16_t small;
        uint16_t normal;
        uint16_t large;
        uint16_t xlarge;
        uint16_t huge;
    } size;
    lv_font_t *small;
    lv_font_t *normal;
    lv_font_t *large;
    lv_font_t *xlarge;
    lv_font_t *huge;
} app_ui_fontset_t;

void app_ui_fontset_set_default_size(const app_ui_t *ui, app_ui_fontset_t *set);

void app_ui_fontset_init_mem(app_ui_fontset_t *set, const char *name, const void *mem, size_t size);

void app_ui_fontset_deinit(app_ui_fontset_t *set);
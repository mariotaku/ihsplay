#pragma once

#include <stdint.h>
#include "lvgl.h"

typedef struct app_t app_t;
typedef struct app_ui_t app_ui_t;

typedef struct app_ui_font_sizes_t {
    uint16_t heading1;
    uint16_t heading2;
    uint16_t heading3;
    uint16_t small;
    uint16_t body;
} app_ui_font_sizes_t;

typedef struct app_ui_fontset_t {
    struct {
        uint16_t heading1;
        uint16_t heading2;
        uint16_t heading3;
        uint16_t body;
        uint16_t small;
        uint16_t huge;
    } sizes;
    lv_font_t *heading1;
    lv_font_t *heading2;
    lv_font_t *heading3;
    lv_font_t *body;
    lv_font_t *small;
    lv_font_t *huge;
} app_ui_fontset_t;

void app_ui_fontset_set_default_size(const app_ui_t *ui, app_ui_fontset_t *set);

void app_ui_fontset_init_mem(app_ui_fontset_t *set, const char *name, const void *mem, size_t size);

void app_ui_fontset_init_fc(app_ui_fontset_t *set, const char *name);

void app_ui_fontset_deinit(app_ui_fontset_t *set);
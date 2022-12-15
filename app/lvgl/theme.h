#pragma once

#include <lvgl.h>

typedef struct app_ui_t app_ui_t;

void app_theme_init(lv_theme_t *theme);

void app_theme_set_ui(lv_theme_t *theme, app_ui_t *ui);

void app_theme_deinit(lv_theme_t *theme);
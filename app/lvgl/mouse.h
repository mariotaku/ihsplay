#pragma once

#include <lvgl.h>

lv_indev_t *app_lv_mouse_indev_init();

void app_lv_mouse_indev_deinit(lv_indev_t *indev);
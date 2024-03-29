#pragma once

#include "lvgl.h"

typedef struct app_ui_t app_ui_t;

void msgbox_inject_nav(app_ui_t *ui, lv_obj_t *obj);

void msgbox_fix_sizes(lv_obj_t *obj, const char *btn_texts[]);
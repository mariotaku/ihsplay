#pragma once

#include "lvgl.h"


void lv_dir_focus_register();

void lv_obj_set_dir_focus_obj(lv_obj_t *obj, lv_dir_t dir, const lv_obj_t *to_focus);

bool lv_obj_focus_dir(lv_obj_t *obj, lv_dir_t dir);

bool lv_obj_focus_dir_by_key(lv_obj_t *obj, lv_key_t dir);
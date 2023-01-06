#pragma once

#include <lvgl.h>

extern const lv_fragment_class_t launcher_fragment_class;

void launcher_fragment_focus_actions(lv_fragment_t *self);

void launcher_fragment_focus_content(lv_fragment_t *self);

void launcher_fragment_open_home(lv_fragment_t *self);

bool launcher_fragment_is_home(lv_fragment_t *self);
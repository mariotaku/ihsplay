#pragma once

#include <lvgl.h>

extern const lv_fragment_class_t launcher_fragment_class;

void launcher_fragment_set_selected_host(lv_fragment_t *self, uint64_t client_id);
#pragma once

#include "lvgl.h"

lv_obj_t *progress_dialog_create(const char *message);

void progress_dialog_set_message(lv_obj_t *obj, const char *message);
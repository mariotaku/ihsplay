#pragma once

#include "lvgl.h"

typedef struct conn_error_fragment_data {
    const char *message;
} conn_error_fragment_data;

extern const lv_fragment_class_t conn_error_fragment_class;
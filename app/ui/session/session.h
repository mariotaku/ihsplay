#pragma once

#include "lvgl.h"
#include "ihslib.h"

typedef struct session_fragment_args_t {
    IHS_HostInfo host;
    IHS_SessionInfo session;
} session_fragment_args_t;

extern const lv_fragment_class_t session_fragment_class;

lv_style_t *session_fragment_get_overlay_style(lv_fragment_t *fragment);

const char *session_fragment_get_host_name(lv_fragment_t *fragment);
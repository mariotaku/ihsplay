#include "wiki.h"

static lv_obj_t *create_obj(lv_fragment_t *self, lv_obj_t *parent);

const lv_fragment_class_t feedback_fragment_class = {
        .create_obj_cb = create_obj,
        .instance_size = sizeof(lv_fragment_t)
};

static lv_obj_t *create_obj(lv_fragment_t *self, lv_obj_t *parent) {
    lv_obj_t *content = lv_obj_create(parent);
    lv_obj_set_size(content, LV_PCT(100), LV_PCT(100));
    lv_obj_set_layout(content, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(content, LV_FLEX_FLOW_COLUMN);

    lv_obj_t *hint1 = lv_label_create(content);
    lv_label_set_text(hint1, "Please include information below in the issue.");
    return content;
}
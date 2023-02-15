#include "app.h"
#include "basic.h"
#include "widgets.h"

#include "config.h"

typedef struct basic_fragment {
    lv_fragment_t base;
    app_t *app;
} basic_fragment;

static void constructor(lv_fragment_t *self, void *arg) {

}

static void destructor(lv_fragment_t *self) {

}

static lv_obj_t *create_obj(lv_fragment_t *self, lv_obj_t *container) {
    lv_obj_t *list = lv_obj_create(container);
    lv_obj_remove_style_all(list);
    lv_obj_set_size(list, LV_PCT(100), LV_PCT(100));
    lv_obj_set_layout(list, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(list, LV_FLEX_FLOW_ROW_WRAP);
    lv_obj_set_style_pad_ver(list, LV_DPX(15), 0);
    lv_obj_set_style_pad_hor(list, LV_DPX(20), 0);
    lv_obj_set_style_pad_gap(list, LV_DPX(10), 0);

#if IHSPLAY_WIP_FEATURES
    settings_select_create(list, "Resolution");
    settings_select_create(list, "Framerate");
    settings_select_create(list, "Bitrate");
#endif
    return list;
}

static void obj_created(lv_fragment_t *self, lv_obj_t *obj) {

}

static void obj_will_delete(lv_fragment_t *self, lv_obj_t *obj) {

}

static void obj_deleted(lv_fragment_t *self, lv_obj_t *obj) {

}

const lv_fragment_class_t settings_basic_fragment_class = {
        .constructor_cb = constructor,
        .destructor_cb = destructor,
        .create_obj_cb = create_obj,
        .obj_created_cb = obj_created,
        .obj_will_delete_cb = obj_will_delete,
        .obj_deleted_cb = obj_deleted,
        .instance_size = sizeof(basic_fragment),
};
#include "settings.h"

#include "app.h"
#include "ui/launcher.h"

typedef struct settings_fragment {
    lv_fragment_t base;
    app_t *app;
} settings_fragment;

static void constructor(lv_fragment_t *self, void *arg);

static void destructor(lv_fragment_t *self);

static lv_obj_t *create_obj(lv_fragment_t *self, lv_obj_t *container);

static void obj_created(lv_fragment_t *self, lv_obj_t *obj);

static void obj_will_delete(lv_fragment_t *self, lv_obj_t *obj);

static void obj_deleted(lv_fragment_t *self, lv_obj_t *obj);

static bool event_cb(lv_fragment_t *self, int code, void *data);

const lv_fragment_class_t settings_fragment_class = {
        .constructor_cb = constructor,
        .destructor_cb = destructor,
        .create_obj_cb = create_obj,
        .obj_created_cb = obj_created,
        .obj_will_delete_cb = obj_will_delete,
        .obj_deleted_cb = obj_deleted,
        .event_cb = event_cb,
        .instance_size = sizeof(settings_fragment),
};

static void constructor(lv_fragment_t *self, void *arg) {
    settings_fragment *fragment = (settings_fragment *) self;
    fragment->app = arg;
}

static void destructor(lv_fragment_t *self) {
}

static lv_obj_t *create_obj(lv_fragment_t *self, lv_obj_t *container) {
    settings_fragment *fragment = (settings_fragment *) self;
    lv_obj_t *tabs = lv_tabview_create(container, LV_DIR_TOP, LV_DPX(40));
    lv_obj_set_style_bg_color(tabs, lv_color_white(), 0);
    lv_obj_set_style_bg_opa(tabs, LV_OPA_20, 0);
    lv_obj_t *basic_tab = lv_tabview_add_tab(tabs, "Basic");
    lv_obj_t *label1 = lv_label_create(basic_tab);
    lv_label_set_text_static(label1, "Basic");
    lv_tabview_add_tab(tabs, "Input");
    lv_tabview_add_tab(tabs, "Audio");
    lv_tabview_add_tab(tabs, "Video");
    lv_tabview_add_tab(tabs, "Advanced");
    return tabs;
}

static void obj_created(lv_fragment_t *self, lv_obj_t *obj) {
    LV_UNUSED(obj);
}

static void obj_will_delete(lv_fragment_t *self, lv_obj_t *obj) {
    LV_UNUSED(obj);
}

static void obj_deleted(lv_fragment_t *self, lv_obj_t *obj) {
    LV_UNUSED(obj);
}

static bool event_cb(lv_fragment_t *self, int code, void *data) {
    (void) data;
    if (code == APP_UI_NAV_BACK) {
        launcher_fragment_open_home(lv_fragment_get_parent(self));
        return true;
    }
    return false;
}
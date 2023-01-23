#include "settings.h"

#include "app.h"
#include "ui/launcher.h"
#include "lvgl/theme.h"
#include "basic.h"
#include "ui/app_ui.h"

typedef struct settings_fragment {
    lv_fragment_t base;
    app_t *app;
    lv_obj_t *content;
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
    fragment->app = ((app_ui_fragment_args_t *) arg)->app;
}

static void destructor(lv_fragment_t *self) {
}

static lv_obj_t *create_obj(lv_fragment_t *self, lv_obj_t *container) {
    settings_fragment *fragment = (settings_fragment *) self;
    lv_obj_t *win = app_lv_win_create(container);
    lv_win_add_title(win, "Settings");
    fragment->content = lv_win_get_content(win);
    return win;
}

static void obj_created(lv_fragment_t *self, lv_obj_t *obj) {
    settings_fragment *fragment = (settings_fragment *) self;
    lv_fragment_t *f = lv_fragment_create(&settings_basic_fragment_class, NULL);
    lv_fragment_manager_replace(self->child_manager, f, &fragment->content);
}

static void obj_will_delete(lv_fragment_t *self, lv_obj_t *obj) {
    LV_UNUSED(obj);
}

static void obj_deleted(lv_fragment_t *self, lv_obj_t *obj) {
    LV_UNUSED(obj);
}

static bool event_cb(lv_fragment_t *self, int code, void *data) {
    (void) data;
    settings_fragment *fragment = (settings_fragment *) self;
    if (code == APP_UI_NAV_BACK) {
        app_ui_pop_top_fragment(fragment->app->ui);
        return true;
    }
    return false;
}
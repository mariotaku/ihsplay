#include "conn_error_fragment.h"
#include "ui/app_ui.h"

typedef struct conn_error_fragment_t {
    lv_fragment_t base;
    app_t *app;
    char *error_message;
} conn_error_fragment_t;

static void conn_error_ctor(lv_fragment_t *self, void *arg);

static void conn_error_dtor(lv_fragment_t *self);

static lv_obj_t *conn_error_create_obj(lv_fragment_t *self, lv_obj_t *container);

static void conn_error_obj_created(lv_fragment_t *self, lv_obj_t *obj);

const lv_fragment_class_t conn_error_fragment_class = {
        .constructor_cb = conn_error_ctor,
        .destructor_cb = conn_error_dtor,
        .create_obj_cb = conn_error_create_obj,
        .obj_created_cb = conn_error_obj_created,
        .instance_size = sizeof(conn_error_fragment_t)
};

static void conn_error_ctor(lv_fragment_t *self, void *arg) {
    conn_error_fragment_t *fragment = (conn_error_fragment_t *) self;
    app_ui_fragment_args_t *args = arg;
    fragment->app = args->app;
    conn_error_fragment_data *data = args->data;
    fragment->error_message = strdup(data->message);
}

static void conn_error_dtor(lv_fragment_t *self) {
    conn_error_fragment_t *fragment = (conn_error_fragment_t *) self;
    free(fragment->error_message);
}

static lv_obj_t *conn_error_create_obj(lv_fragment_t *self, lv_obj_t *container) {
    conn_error_fragment_t *fragment = (conn_error_fragment_t *) self;
    lv_obj_t *obj = lv_obj_create(container);
    lv_obj_set_style_pad_gap(obj, LV_DPX(10), 0);
    lv_obj_set_flex_flow(obj, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(obj, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    lv_obj_t *conn_error_label = lv_label_create(obj);
    lv_label_set_text(conn_error_label, fragment->error_message);
    return obj;
}

static void conn_error_obj_created(lv_fragment_t *self, lv_obj_t *obj) {
}
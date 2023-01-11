#include "pin_fragment.h"
#include "ui/app_ui.h"

typedef struct pin_fragment_t {
    lv_fragment_t base;
    app_t *app;
    char pin[8];
} pin_fragment_t;

static void pin_ctor(lv_fragment_t *self, void *arg);

static lv_obj_t *pin_create_obj(lv_fragment_t *self, lv_obj_t *container);

const lv_fragment_class_t pin_fragment_class = {
        .constructor_cb = pin_ctor,
        .create_obj_cb = pin_create_obj,
        .instance_size = sizeof(pin_fragment_t)
};

static void pin_ctor(lv_fragment_t *self, void *arg) {
    pin_fragment_t *fragment = (pin_fragment_t *) self;
    app_ui_fragment_args_t *args = arg;
    fragment->app = args->app;
    strncpy(fragment->pin, args->data, sizeof(fragment->pin) - 1);
}

static lv_obj_t *pin_create_obj(lv_fragment_t *self, lv_obj_t *container) {
    pin_fragment_t *fragment = (pin_fragment_t *) self;
    lv_obj_t *obj = lv_obj_create(container);
    lv_obj_t *pin_label = lv_label_create(obj);
    lv_obj_set_style_text_font(pin_label, fragment->app->ui->font.huge, 0);
    lv_label_set_text(pin_label, fragment->pin);
    return obj;
}
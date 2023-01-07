#include "cec_key.h"

#include "app.h"
#include "ui/app_ui.h"

#include "logging/app_logging.h"
#include "cec_support.h"
#include "lvgl/keypad.h"

typedef struct key_event_t {
    lv_key_t key;
    bool pressed;
} key_event_t;

static void cb_key_main(app_t *app, void *data);

static lv_key_t key_from_cec(cec_user_control_code code);

void cec_support_cb_key(void *cbparam, const cec_keypress *keypress) {
    cec_support_ctx_t *support = cbparam;
    lv_key_t key = key_from_cec(keypress->keycode);
    if (key == 0) {
        return;
    }
    key_event_t *data = malloc(sizeof(key_event_t));
    data->key = key;
    data->pressed = keypress->duration == 0;
    app_run_on_main(support->app, cb_key_main, data);
}

static void cb_key_main(app_t *app, void *data) {
    key_event_t *ev = data;
    app_indev_keypad_inject_key(app->ui->indev.keypad, ev->key, ev->pressed);
    free(data);
}

static lv_key_t key_from_cec(cec_user_control_code code) {
    switch (code) {
        case CEC_USER_CONTROL_CODE_SELECT:
            return LV_KEY_ENTER;
        case CEC_USER_CONTROL_CODE_UP:
            return LV_KEY_UP;
        case CEC_USER_CONTROL_CODE_DOWN:
            return LV_KEY_DOWN;
        case CEC_USER_CONTROL_CODE_LEFT:
            return LV_KEY_LEFT;
        case CEC_USER_CONTROL_CODE_RIGHT:
            return LV_KEY_RIGHT;
        case CEC_USER_CONTROL_CODE_EXIT:
            return LV_KEY_ESC;
        default:
            return 0;
    }
}
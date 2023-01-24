#include "wiki.h"
#include "ss4s.h"
#include "config.h"
#include "app.h"

typedef struct feedback_fragment_t {
    lv_fragment_t base;
    app_t *app;
} feedback_fragment_t;

static void feedback_ctor(lv_fragment_t *self, void *arg);

static lv_obj_t *create_obj(lv_fragment_t *self, lv_obj_t *parent);

const lv_fragment_class_t feedback_fragment_class = {
        .constructor_cb = feedback_ctor,
        .create_obj_cb = create_obj,
        .instance_size = sizeof(feedback_fragment_t)
};

static void feedback_ctor(lv_fragment_t *self, void *arg) {
    ((feedback_fragment_t *) self)->app = arg;
}

static lv_obj_t *create_obj(lv_fragment_t *self, lv_obj_t *parent) {
    app_t *app = ((feedback_fragment_t *) self)->app;
    lv_obj_t *content = lv_obj_create(parent);
    lv_obj_set_size(content, LV_PCT(100), LV_PCT(100));
    lv_obj_set_layout(content, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(content, LV_FLEX_FLOW_ROW_WRAP);

    lv_obj_t *hint1 = lv_label_create(content);
    lv_obj_set_width(hint1, LV_PCT(100));
    lv_label_set_text(hint1, "Please include information below when you send feedback. "
                             "You can also scan the QR code to copy the text.");

    lv_obj_t *qrcode = lv_qrcode_create(content, LV_DPX(200), lv_color_black(), lv_color_white());
    lv_obj_set_style_pad_all(qrcode, LV_DPX(5), 0);
    lv_obj_set_style_bg_opa(qrcode, LV_OPA_COVER, 0);
    lv_obj_set_style_bg_color(qrcode, lv_color_white(), 0);

    static const char *info_fmt = "Version: %s\n"
                                  "Audio Module: %s\n"
                                  "Video Module: %s\n"
                                  "%s Version: %s\n";
    lv_obj_t *info_label = lv_label_create(content);
    char *os_version_str = version_info_str(&app->os_info.version);
    lv_label_set_text_fmt(info_label, info_fmt, IHSPLAY_VERSION_STRING, SS4S_GetAudioModuleName(),
                          SS4S_GetVideoModuleName(), app->os_info.name, os_version_str);
    if (os_version_str != NULL) {
        free(os_version_str);
    }
    lv_obj_set_flex_grow(info_label, 1);

    const char *info_txt = lv_label_get_text(info_label);
    lv_qrcode_update(qrcode, info_txt, strlen(info_txt));

    return content;
}
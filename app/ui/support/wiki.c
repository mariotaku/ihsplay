#include "wiki.h"

static lv_obj_t *create_obj(lv_fragment_t *self, lv_obj_t *parent);

const lv_fragment_class_t wiki_fragment_class = {
        .create_obj_cb = create_obj,
        .instance_size = sizeof(lv_fragment_t)
};

static lv_obj_t *create_obj(lv_fragment_t *self, lv_obj_t *parent) {
    lv_obj_t *content = lv_obj_create(parent);
    lv_obj_set_size(content, LV_PCT(100), LV_PCT(100));
    lv_obj_set_layout(content, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(content, LV_FLEX_FLOW_COLUMN);
    lv_obj_t *hint = lv_label_create(content);
    lv_obj_set_size(hint, LV_PCT(100), LV_SIZE_CONTENT);
    const char *url = "https://github.com/mariotaku/ihsplay/wiki";

    lv_label_set_text_fmt(hint, "Open %s for guides and frequently asked questions. Or scan the QR code below.", url);
    lv_obj_t *qrcode = lv_qrcode_create(content, LV_DPX(200), lv_color_black(), lv_color_white());
    lv_qrcode_update(qrcode, url, strlen(url));

    return content;
}
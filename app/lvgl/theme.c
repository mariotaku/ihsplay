
#include "theme.h"

static void apply_cb(lv_theme_t *theme, lv_obj_t *obj);

static struct {
    lv_style_t scr;
    lv_style_t obj;
    lv_style_t btn;
    lv_style_t btn_pressed;
} styles;

void app_theme_init(lv_theme_t *theme) {
    lv_theme_set_apply_cb(theme, apply_cb);

    lv_style_init(&styles.scr);
    lv_style_set_bg_color(&styles.scr, lv_color_make(0, 0, 0xFF));
    lv_style_set_bg_opa(&styles.scr, LV_OPA_COVER);

    lv_style_init(&styles.obj);
    lv_style_set_text_color(&styles.obj, lv_color_white());

    lv_style_init(&styles.btn);
    lv_style_set_pad_all(&styles.btn, LV_DPX(10));
    lv_style_set_bg_color(&styles.btn, lv_color_white());
    lv_style_set_bg_opa(&styles.btn, LV_OPA_20);

    lv_style_init(&styles.btn_pressed);
    lv_style_set_bg_color(&styles.btn_pressed, lv_color_darken(lv_color_white(), LV_OPA_20));
    lv_style_set_bg_opa(&styles.btn_pressed, LV_OPA_20);
}

static void apply_cb(lv_theme_t *theme, lv_obj_t *obj) {
    if (lv_obj_get_parent(obj) == NULL) {
        lv_obj_add_style(obj, &styles.scr, 0);
        return;
    }
    lv_obj_add_style(obj, &styles.obj, 0);
    if (lv_obj_has_class(obj, &lv_btn_class)) {
        lv_obj_add_style(obj, &styles.btn, 0);
        lv_obj_add_style(obj, &styles.btn_pressed, LV_STATE_PRESSED);
    }
}
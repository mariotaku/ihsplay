
#include "theme.h"

#include "ext/msgbox_ext.h"

static void apply_cb(lv_theme_t *theme, lv_obj_t *obj);

typedef struct theme_context_t {
    app_ui_t *ui;

    struct {
        const lv_font_t *h6;
        const lv_font_t *body1;
        const lv_font_t *button;
    } fonts;

    lv_style_t scr;
    lv_style_t obj;
    lv_style_t btn;
    lv_style_t btn_focused;
    lv_style_t btn_pressed;

    lv_style_t modal_bg;

    lv_style_t msgbox;
    lv_style_t msgbox_backdrop;
    lv_style_t msgbox_title;
    lv_style_t msgbox_text;
    lv_style_t msgbox_btns;
    lv_style_t msgbox_btns_item;
    lv_style_t msgbox_btns_item_focused;
    lv_style_t msgbox_btns_item_pressed;

    lv_style_t arc_indic;
    lv_style_t arc_indic_primary;

} theme_context_t;

void app_theme_init(lv_theme_t *theme) {
    theme_context_t *styles = lv_mem_alloc(sizeof(theme_context_t));
    lv_memset_00(styles, sizeof(theme_context_t));

    styles->fonts.h6 = &lv_font_montserrat_42;
    styles->fonts.body1 = &lv_font_montserrat_34;
    styles->fonts.button = &lv_font_montserrat_30;

    theme->font_small = &lv_font_montserrat_28;
    theme->font_normal = &lv_font_montserrat_32;
    theme->font_large = &lv_font_montserrat_42;

    lv_color_t primary_color = lv_color_hex(0x1387b8);
    lv_color_t accent_color = lv_color_lighten(primary_color, 1);

    theme->color_primary = primary_color;

    lv_style_init(&styles->scr);
    lv_style_set_bg_color(&styles->scr, lv_color_black());
    lv_style_set_bg_opa(&styles->scr, LV_OPA_COVER);

    lv_style_init(&styles->obj);
    lv_style_set_text_color(&styles->obj, lv_color_white());
    lv_style_set_pad_gap(&styles->obj, LV_DPX(10));

    lv_style_init(&styles->btn);
    lv_style_set_pad_all(&styles->btn, LV_DPX(10));
    lv_style_set_bg_color(&styles->btn, lv_color_white());
    lv_style_set_bg_opa(&styles->btn, LV_OPA_20);

    lv_style_init(&styles->btn_focused);
    lv_style_set_bg_color(&styles->btn_focused, lv_color_mix(primary_color, lv_color_white(), LV_OPA_10));

    lv_style_init(&styles->btn_pressed);
    lv_style_set_bg_color(&styles->btn_pressed, lv_color_mix(primary_color, lv_color_white(), LV_OPA_20));

    lv_style_init(&styles->modal_bg);
    lv_style_set_bg_color(&styles->modal_bg, lv_color_hex(0x25282e));
    lv_style_set_bg_opa(&styles->modal_bg, LV_OPA_COVER);
    lv_style_set_shadow_color(&styles->modal_bg, lv_color_black());
    lv_style_set_shadow_opa(&styles->modal_bg, LV_OPA_30);
    lv_style_set_shadow_width(&styles->modal_bg, LV_DPX(20));
    lv_style_set_shadow_ofs_y(&styles->modal_bg, LV_DPX(10));
    lv_style_set_radius(&styles->modal_bg, LV_DPX(5));
    lv_style_set_min_width(&styles->modal_bg, LV_PCT(40));
    lv_style_set_max_width(&styles->modal_bg, LV_PCT(60));

    lv_style_init(&styles->msgbox);
    lv_style_set_pad_top(&styles->msgbox, LV_DPX(16));
    lv_style_set_pad_bottom(&styles->msgbox, LV_DPX(8));
    lv_style_set_pad_hor(&styles->msgbox, LV_DPX(8));
    lv_style_set_flex_main_place(&styles->msgbox, LV_FLEX_ALIGN_END);

    lv_style_init(&styles->msgbox_backdrop);
    lv_style_set_bg_color(&styles->msgbox_backdrop, lv_color_black());
    lv_style_set_bg_opa(&styles->msgbox_backdrop, LV_OPA_30);

    lv_style_init(&styles->msgbox_title);
    lv_style_set_text_font(&styles->msgbox_title, styles->fonts.h6);
    lv_style_set_pad_hor(&styles->msgbox_title, LV_DPX(16));

    lv_style_init(&styles->msgbox_text);
    lv_style_set_text_font(&styles->msgbox_text, styles->fonts.body1);
    lv_style_set_pad_top(&styles->msgbox_text, LV_DPX(8));
    lv_style_set_pad_hor(&styles->msgbox_text, LV_DPX(16));
    lv_style_set_pad_bottom(&styles->msgbox_text, LV_DPX(16));

    lv_style_init(&styles->msgbox_btns);

    lv_style_init(&styles->msgbox_btns_item);
    lv_style_set_radius(&styles->msgbox_btns_item, LV_DPX(2));
    lv_style_set_text_font(&styles->msgbox_btns_item, styles->fonts.button);
    lv_style_set_text_color(&styles->msgbox_btns_item, accent_color);

    lv_style_init(&styles->msgbox_btns_item_focused);
    lv_style_set_bg_color(&styles->msgbox_btns_item_focused, primary_color);
    lv_style_set_bg_opa(&styles->msgbox_btns_item_focused, LV_OPA_10);

    lv_style_init(&styles->msgbox_btns_item_pressed);
    lv_style_set_bg_color(&styles->msgbox_btns_item_pressed, primary_color);
    lv_style_set_bg_opa(&styles->msgbox_btns_item_pressed, LV_OPA_20);

    lv_style_init(&styles->arc_indic);
    lv_style_set_arc_color(&styles->arc_indic, lv_color_lighten(accent_color, LV_OPA_80));
    lv_style_set_arc_opa(&styles->arc_indic, LV_OPA_20);
    lv_style_set_arc_width(&styles->arc_indic, LV_DPX(15));
    lv_style_set_arc_rounded(&styles->arc_indic, true);

    lv_style_init(&styles->arc_indic_primary);
    lv_style_set_arc_color(&styles->arc_indic_primary, accent_color);
    lv_style_set_arc_opa(&styles->arc_indic_primary, LV_OPA_COVER);

    theme->user_data = styles;

    lv_theme_set_apply_cb(theme, apply_cb);
}

void app_theme_set_ui(lv_theme_t *theme, app_ui_t *ui) {
    theme_context_t *styles = theme->user_data;
    styles->ui = ui;
}

void app_theme_deinit(lv_theme_t *theme) {
    theme_context_t *styles = theme->user_data;
    lv_style_reset(&styles->msgbox_btns_item_pressed);
    lv_style_reset(&styles->msgbox_btns_item_focused);
    lv_style_reset(&styles->msgbox_btns_item);
    lv_style_reset(&styles->msgbox_btns);
    lv_style_reset(&styles->msgbox_text);
    lv_style_reset(&styles->msgbox_title);
    lv_style_reset(&styles->msgbox_backdrop);
    lv_style_reset(&styles->msgbox);
    lv_style_reset(&styles->modal_bg);
    lv_style_reset(&styles->btn_focused);
    lv_style_reset(&styles->btn_pressed);
    lv_style_reset(&styles->btn);
    lv_style_reset(&styles->obj);
    lv_style_reset(&styles->scr);

    free(styles);
}

static void apply_cb(lv_theme_t *theme, lv_obj_t *obj) {
    theme_context_t *styles = theme->user_data;
    lv_obj_t *parent = lv_obj_get_parent(obj);
    if (parent == NULL) {
        lv_obj_add_style(obj, &styles->scr, 0);
        return;
    }
    lv_obj_add_style(obj, &styles->obj, 0);
    if (lv_obj_has_class(obj, &lv_btn_class)) {
        lv_obj_add_style(obj, &styles->btn, 0);
        lv_obj_add_style(obj, &styles->btn_pressed, LV_STATE_PRESSED);
        lv_obj_add_style(obj, &styles->btn_focused, LV_STATE_FOCUSED);
        lv_obj_add_style(obj, &styles->btn_focused, LV_STATE_FOCUS_KEY);
    } else if (lv_obj_has_class(obj, &lv_label_class)) {
        if (lv_obj_check_type(parent, &lv_msgbox_class)) {
            if (lv_msgbox_get_title(parent) == NULL && lv_msgbox_get_content(parent) == NULL) {
                // Title was not assigned, and the content was not created either. Assume this is the title
                lv_obj_add_style(obj, &styles->msgbox_title, 0);
            }
        } else if (lv_obj_check_type(parent->parent, &lv_msgbox_class)) {
            if (lv_msgbox_get_content(parent->parent) == lv_obj_get_parent(obj)) {
                lv_obj_add_style(obj, &styles->msgbox_text, 0);
            }
        }
    } else if (lv_obj_has_class(obj, &lv_btnmatrix_class)) {
        if (lv_obj_check_type(parent, &lv_msgbox_class)) {
            lv_obj_add_style(obj, &styles->msgbox_btns, 0);
            lv_obj_add_style(obj, &styles->msgbox_btns_item, LV_PART_ITEMS);
            lv_obj_add_style(obj, &styles->msgbox_btns_item_focused, LV_PART_ITEMS | LV_STATE_FOCUSED);
            lv_obj_add_style(obj, &styles->msgbox_btns_item_pressed, LV_PART_ITEMS | LV_STATE_PRESSED);
        }
    } else if (lv_obj_check_type(obj, &lv_msgbox_class)) {
        lv_obj_add_style(obj, &styles->modal_bg, 0);
        lv_obj_add_style(obj, &styles->msgbox, 0);
        msgbox_inject_nav(styles->ui, obj);
    } else if (lv_obj_check_type(obj, &lv_msgbox_backdrop_class)) {
        lv_obj_add_style(obj, &styles->msgbox_backdrop, 0);
    } else if (lv_obj_check_type(obj, &lv_spinner_class)) {
        lv_obj_add_style(obj, &styles->arc_indic, 0);
        lv_obj_add_style(obj, &styles->arc_indic, LV_PART_INDICATOR);
        lv_obj_add_style(obj, &styles->arc_indic_primary, LV_PART_INDICATOR);
    }
}

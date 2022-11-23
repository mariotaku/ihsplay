#include "widgets.h"

lv_obj_t *settings_select_create(lv_obj_t *parent, const char *name) {
    lv_obj_t *item = lv_label_create(parent);
    lv_label_set_text(item, name);
    lv_obj_set_width(item, LV_PCT(100));

    lv_obj_t *dropdown = lv_dropdown_create(item);
    lv_obj_set_width(dropdown, LV_PCT(50));
    lv_obj_align(dropdown, LV_ALIGN_RIGHT_MID, 0, 0);

    return item;
}
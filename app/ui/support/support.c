#include "support.h"
#include "wiki.h"
#include "feedback.h"

typedef struct support_fragment_t {
    lv_fragment_t base;
    lv_coord_t col_dsc[3], row_dsc[4];
} support_fragment_t;

static void constructor(lv_fragment_t *self, void *args);

static lv_obj_t *create_obj(lv_fragment_t *self, lv_obj_t *parent);

static void obj_created(lv_fragment_t *self, lv_obj_t *obj);

static void show_page(lv_fragment_t *self, const lv_fragment_class_t *cls);

static lv_obj_t *add_btn(lv_obj_t *parent, const char *text, int index, const lv_fragment_class_t *cls);

static void btn_click_cb(lv_event_t *e);

const lv_fragment_class_t support_fragment_class = {
        .constructor_cb = constructor,
        .create_obj_cb = create_obj,
        .obj_created_cb = obj_created,
        .instance_size = sizeof(support_fragment_t)
};

static void constructor(lv_fragment_t *self, void *args) {
    support_fragment_t *fragment = (support_fragment_t *) self;
    fragment->col_dsc[0] = LV_DPX(200);
    fragment->col_dsc[1] = LV_GRID_FR(1);
    fragment->col_dsc[2] = LV_GRID_TEMPLATE_LAST;
    fragment->row_dsc[0] = LV_DPX(40);
    fragment->row_dsc[1] = LV_DPX(40);
    fragment->row_dsc[2] = LV_GRID_FR(1);
    fragment->row_dsc[3] = LV_GRID_TEMPLATE_LAST;
}

static lv_obj_t *create_obj(lv_fragment_t *self, lv_obj_t *parent) {
    support_fragment_t *fragment = (support_fragment_t *) self;
    lv_obj_t *content = lv_obj_create(parent);
    lv_obj_set_size(content, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_pad_hor(content, LV_DPX(30), 0);
    lv_obj_set_style_pad_ver(content, LV_DPX(15), 0);
    lv_obj_set_style_pad_row(content, LV_DPX(15), 0);
    lv_obj_set_style_pad_column(content, LV_DPX(30), 0);
    lv_obj_set_layout(content, LV_LAYOUT_GRID);
    lv_obj_set_grid_dsc_array(content, fragment->col_dsc, fragment->row_dsc);
    lv_obj_add_event_cb(content, btn_click_cb, LV_EVENT_CLICKED, fragment);

    add_btn(content, "Get Help", 0, &wiki_fragment_class);
    add_btn(content, "Feedback", 1, &feedback_fragment_class);

    return content;
}

static void obj_created(lv_fragment_t *self, lv_obj_t *obj) {
    show_page(self, &wiki_fragment_class);
}


static lv_obj_t *add_btn(lv_obj_t *parent, const char *text, int index, const lv_fragment_class_t *cls) {
    lv_obj_t *btn = lv_btn_create(parent);
    lv_obj_t *label = lv_label_create(btn);
    lv_label_set_text(label, text);
    lv_obj_set_grid_cell(btn, LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_STRETCH, index, 1);
    lv_obj_set_user_data(btn, (void *) cls);
    lv_obj_add_flag(btn, LV_OBJ_FLAG_EVENT_BUBBLE);
    return btn;
}

static void show_page(lv_fragment_t *self, const lv_fragment_class_t *cls) {
    lv_fragment_t *page = lv_fragment_create(cls, NULL);
    lv_fragment_manager_replace(self->child_manager, page, &self->obj);
    lv_obj_set_grid_cell(page->obj, LV_GRID_ALIGN_STRETCH, 1, 1, LV_GRID_ALIGN_STRETCH, 0, 3);
}

static void btn_click_cb(lv_event_t *e) {
    lv_obj_t *target = lv_event_get_target(e);
    if (!lv_obj_check_type(target, &lv_btn_class)) {
        return;
    }
    const lv_fragment_class_t *cls = lv_obj_get_user_data(target);
    lv_fragment_t *self = lv_event_get_user_data(e);
    show_page(self, cls);
}
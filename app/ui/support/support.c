#include "support.h"
#include "wiki.h"
#include "feedback.h"
#include "app.h"

typedef struct support_fragment_t {
    lv_fragment_t base;
    lv_coord_t col_dsc[3], row_dsc[4];
    lv_obj_t *win_content;
    int num_btns;
} support_fragment_t;

static void constructor(lv_fragment_t *self, void *args);

static lv_obj_t *create_obj(lv_fragment_t *self, lv_obj_t *parent);

static void obj_created(lv_fragment_t *self, lv_obj_t *obj);

static bool event_cb(lv_fragment_t *self, int code, void *data);

static void show_page(lv_fragment_t *self, const lv_fragment_class_t *cls);

static lv_obj_t *add_btn(support_fragment_t *fragment, lv_obj_t *parent, const char *text,
                         const lv_fragment_class_t *cls);

static void btn_click_cb(lv_event_t *e);

static void btn_key_cb(lv_event_t *e);

const lv_fragment_class_t support_fragment_class = {
        .constructor_cb = constructor,
        .create_obj_cb = create_obj,
        .obj_created_cb = obj_created,
        .event_cb = event_cb,
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
    fragment->num_btns = 0;
}

static lv_obj_t *create_obj(lv_fragment_t *self, lv_obj_t *parent) {
    support_fragment_t *fragment = (support_fragment_t *) self;
    lv_obj_t *win = lv_win_create(parent, LV_SIZE_CONTENT);
    lv_win_add_title(win, "Support");
    lv_obj_set_size(win, LV_PCT(100), LV_PCT(100));
    fragment->win_content = lv_win_get_content(win);
    lv_obj_set_style_pad_row(fragment->win_content, LV_DPX(15), 0);
    lv_obj_set_style_pad_column(fragment->win_content, LV_DPX(30), 0);
    lv_obj_set_layout(fragment->win_content, LV_LAYOUT_GRID);
    lv_obj_set_grid_dsc_array(fragment->win_content, fragment->col_dsc, fragment->row_dsc);
    lv_obj_add_event_cb(fragment->win_content, btn_click_cb, LV_EVENT_CLICKED, fragment);
    lv_obj_add_event_cb(fragment->win_content, btn_key_cb, LV_EVENT_KEY, fragment);


    add_btn(fragment, fragment->win_content, "Get Help", &wiki_fragment_class);
    add_btn(fragment, fragment->win_content, "Feedback", &feedback_fragment_class);

    return win;
}

static void obj_created(lv_fragment_t *self, lv_obj_t *obj) {
    show_page(self, &wiki_fragment_class);
}

static bool event_cb(lv_fragment_t *self, int code, void *data) {
    (void) data;
    if (code == APP_UI_NAV_BACK) {
        lv_fragment_manager_pop(lv_fragment_get_manager(self));
        return true;
    }
    return false;
}

static lv_obj_t *add_btn(support_fragment_t *fragment, lv_obj_t *parent, const char *text,
                         const lv_fragment_class_t *cls) {
    LV_ASSERT_NULL(cls);
    lv_obj_t *btn = lv_btn_create(parent);
    lv_obj_t *label = lv_label_create(btn);
    lv_label_set_text(label, text);
    lv_obj_set_grid_cell(btn, LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_STRETCH, fragment->num_btns++, 1);
    lv_obj_set_user_data(btn, (void *) cls);
    lv_obj_add_flag(btn, LV_OBJ_FLAG_EVENT_BUBBLE);
    return btn;
}

static void show_page(lv_fragment_t *self, const lv_fragment_class_t *cls) {
    support_fragment_t *fragment = (support_fragment_t *) self;
    lv_fragment_t *page = lv_fragment_create(cls, NULL);
    lv_fragment_manager_replace(self->child_manager, page, &fragment->win_content);
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

static void btn_key_cb(lv_event_t *e) {
    lv_obj_t *target = lv_event_get_target(e);
    if (!lv_obj_check_type(target, &lv_btn_class)) {
        return;
    }
    lv_fragment_t *self = lv_event_get_user_data(e);
    lv_coord_t pos = lv_obj_get_style_grid_cell_row_pos(target, 0);
    lv_group_t *group = lv_obj_get_group(target);
    switch (lv_event_get_key(e)) {
        case LV_KEY_UP: {
            if (pos > 0) {
                lv_group_focus_prev(group);
            }
            break;
        }
        case LV_KEY_DOWN: {
            support_fragment_t *fragment = (support_fragment_t *) self;
            if (pos < fragment->num_btns - 1) {
                lv_group_focus_next(group);
            }
            break;
        }
    }
}
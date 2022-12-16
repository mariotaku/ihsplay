#include "connection_progress.h"
#include "session.h"
#include "app.h"
#include "ui/app_ui.h"

typedef struct connection_progress_fragment {
    lv_fragment_t base;
    app_t *app;
    lv_coord_t col_dsc[3], row_dsc[3];
} connection_progress_fragment;

void constructor(lv_fragment_t *self, void *args) {
    connection_progress_fragment *fragment = (connection_progress_fragment *) self;
    fragment->app = args;
}

lv_obj_t *create_obj_cb(lv_fragment_t *self, lv_obj_t *container) {
    connection_progress_fragment *fragment = (connection_progress_fragment *) self;
    lv_fragment_t *session_fragment = lv_fragment_get_parent(self);
    lv_obj_t *content = lv_obj_create(container);
    lv_obj_remove_style_all(content);
    lv_obj_add_style(content, session_fragment_get_overlay_style(session_fragment), 0);

    lv_obj_set_layout(content, LV_LAYOUT_GRID);
    fragment->col_dsc[0] = LV_GRID_CONTENT;
    fragment->col_dsc[1] = LV_GRID_FR(1);
    fragment->col_dsc[2] = LV_GRID_TEMPLATE_LAST;

    fragment->row_dsc[0] = LV_GRID_FR(1);
    fragment->row_dsc[1] = LV_DPX(50);
    fragment->row_dsc[2] = LV_GRID_TEMPLATE_LAST;

    lv_obj_set_grid_dsc_array(content, fragment->col_dsc, fragment->row_dsc);

    lv_obj_t *title = lv_label_create(content);
    lv_obj_set_style_text_font(title, lv_theme_get_font_large(content), 0);
    lv_obj_set_grid_cell(title, LV_GRID_ALIGN_START, 0, 1, LV_GRID_ALIGN_START, 0, 1);
    lv_obj_set_style_pad_top(title, LV_DPX(20), 0);
    lv_label_set_text_static(title, "Connecting");

    lv_obj_t *subtitle = lv_label_create(content);
    lv_obj_set_style_pad_top(subtitle, LV_DPX(15), 0);
    lv_obj_set_grid_cell(subtitle, LV_GRID_ALIGN_START, 0, 1, LV_GRID_ALIGN_START, 1, 1);
    lv_label_set_text_fmt(subtitle, "Setting up streaming for %s...", session_fragment_get_host_name(session_fragment));

    lv_obj_t *spinner = lv_spinner_create(content, 1000, 60);
    lv_obj_set_style_arc_width(spinner, LV_DPX(10), 0);
    lv_obj_set_style_arc_width(spinner, LV_DPX(10), LV_PART_INDICATOR);
    lv_obj_set_size(spinner, LV_DPX(50), LV_DPX(50));
    lv_obj_set_grid_cell(spinner, LV_GRID_ALIGN_END, 1, 1, LV_GRID_ALIGN_CENTER, 0, 2);

    return content;
}

const lv_fragment_class_t connection_progress_class = {
        .constructor_cb = constructor,
        .create_obj_cb = create_obj_cb,
        .instance_size = sizeof(connection_progress_fragment),
};
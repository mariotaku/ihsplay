#include <stdlib.h>
#include <lvgl.h>

#include "app_ui.h"

typedef struct app_root_fragment {
    lv_fragment_t base;
    lv_coord_t col_dsc[3], row_dsc[7];
} app_root_fragment;

static void ctor(lv_fragment_t *self, void *arg);

static lv_obj_t *create_obj(lv_fragment_t *self, lv_obj_t *container);

static lv_obj_t *nav_btn_create(lv_obj_t *container, const char *txt);

static const lv_fragment_class_t app_root_fragment_class = {
        .constructor_cb = ctor,
        .create_obj_cb = create_obj,
        .instance_size = sizeof(app_root_fragment),
};

app_ui_t *app_ui_create(app_t *app, lv_disp_t *disp) {
    app_ui_t *ui = calloc(1, sizeof(app_ui_t));
    ui->app = app;
    ui->root = lv_disp_get_scr_act(disp);
    ui->fm = lv_fragment_manager_create(NULL);

    lv_fragment_t *f = lv_fragment_create(&app_root_fragment_class, app);
    lv_fragment_manager_replace(ui->fm, f, &ui->root);
    return ui;
}

void app_ui_destroy(app_ui_t *ui) {
    lv_fragment_manager_del(ui->fm);
    free(ui);
}

static void ctor(lv_fragment_t *self, void *arg) {
    app_root_fragment *fragment = (app_root_fragment *) self;
    fragment->col_dsc[0] = LV_DPX(200);
    fragment->col_dsc[1] = LV_GRID_FR(1);
    fragment->col_dsc[2] = LV_GRID_TEMPLATE_LAST;
    fragment->row_dsc[0] = LV_GRID_CONTENT;
    fragment->row_dsc[1] = LV_GRID_CONTENT;
    fragment->row_dsc[2] = LV_GRID_CONTENT;
    fragment->row_dsc[3] = LV_GRID_CONTENT;
    fragment->row_dsc[4] = LV_GRID_CONTENT;
    fragment->row_dsc[5] = LV_GRID_FR(1);
    fragment->row_dsc[6] = LV_GRID_TEMPLATE_LAST;
}

static lv_obj_t *create_obj(lv_fragment_t *self, lv_obj_t *container) {
    app_root_fragment *fragment = (app_root_fragment *) self;
    lv_obj_t *root = lv_obj_create(container);
    lv_obj_set_layout(root, LV_LAYOUT_GRID);
    lv_obj_set_grid_dsc_array(root, fragment->col_dsc, fragment->row_dsc);

    lv_obj_t *title = lv_label_create(root);
    lv_label_set_text(title, "Steam Link");
    lv_obj_set_size(title, LV_PCT(100), LV_SIZE_CONTENT);
    lv_obj_set_grid_cell(title, LV_GRID_ALIGN_START, 0, 2, LV_GRID_ALIGN_SPACE_AROUND, 0, 1);

    lv_obj_t *subtitle = lv_label_create(root);
    lv_label_set_text(subtitle, "Stream games from your computer with Steam");
    lv_obj_set_size(subtitle, LV_PCT(100), LV_SIZE_CONTENT);
    lv_obj_set_grid_cell(subtitle, LV_GRID_ALIGN_START, 0, 2, LV_GRID_ALIGN_SPACE_AROUND, 1, 1);

    lv_obj_t *btn_servers = nav_btn_create(root, "Start Playing");
    lv_obj_set_grid_cell(btn_servers, LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_SPACE_AROUND, 2, 1);

    lv_obj_t *btn_settings = nav_btn_create(root, "Settings");
    lv_obj_set_grid_cell(btn_settings, LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_SPACE_AROUND, 3, 1);

    lv_obj_t *btn_support = nav_btn_create(root, "Support");
    lv_obj_set_grid_cell(btn_support, LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_SPACE_AROUND, 4, 1);

    lv_obj_t *nav_content = lv_obj_create(root);
    lv_obj_set_style_bg_color(nav_content, lv_color_make(255, 0, 0), 0);
    lv_obj_set_grid_cell(nav_content, LV_GRID_ALIGN_STRETCH, 1, 1, LV_GRID_ALIGN_STRETCH, 2, 4);

    lv_obj_set_size(root, LV_PCT(100), LV_PCT(100));
    return root;
}

static lv_obj_t *nav_btn_create(lv_obj_t *container, const char *txt) {
    lv_obj_t *btn = lv_btn_create(container);
    lv_obj_t *label = lv_label_create(btn);
    lv_label_set_text(label, txt);
    return btn;
}

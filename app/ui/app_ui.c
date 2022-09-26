#include <stdlib.h>

#include <lvgl.h>
#include <src/draw/sdl/lv_draw_sdl.h>

#include "app_ui.h"
#include "launcher.h"
#include "lvgl/fonts/material-icons/regular.h"

app_ui_t *app_ui_create(app_t *app, lv_disp_t *disp) {
    lv_draw_sdl_drv_param_t *param = disp->driver->user_data;
    app_ui_t *ui = calloc(1, sizeof(app_ui_t));
    ui->app = app;
    ui->window = param->user_data;
    ui->root = lv_disp_get_scr_act(disp);
    ui->fm = lv_fragment_manager_create(NULL);

    app_ui_fontset_set_default_size(ui, &ui->iconfont);
    app_ui_fontset_init_mem(&ui->iconfont, "MaterialIcons-Regular", ttf_material_icons_regular_data,
                            ttf_material_icons_regular_size);

    lv_obj_set_style_bg_opa(ui->root, LV_OPA_0, 0);
    return ui;
}

void app_ui_created(app_ui_t *ui) {
    app_ui_push_fragment(ui, &launcher_fragment_class, NULL);
}

void app_ui_destroy(app_ui_t *ui) {
    app_ui_fontset_deinit(&ui->iconfont);
    lv_fragment_manager_del(ui->fm);
    free(ui);
}

void app_ui_push_fragment(app_ui_t *ui, const lv_fragment_class_t *cls, void *args) {
    app_ui_fragment_args_t fargs = {ui->app, args};
    lv_fragment_t *f = lv_fragment_create(cls, &fargs);
    lv_fragment_manager_push(ui->fm, f, &ui->root);
}

void app_ui_pop_fragment(app_ui_t *ui) {
    lv_fragment_manager_pop(ui->fm);
}
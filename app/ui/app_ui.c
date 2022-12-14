#include <stdlib.h>

#include <lvgl.h>
#include <src/draw/sdl/lv_draw_sdl.h>

#include "app_ui.h"
#include "launcher.h"
#include "lvgl/fonts/bootstrap-icons/regular.h"
#include "lvgl/keypad.h"
#include "lvgl/mouse.h"

app_ui_t *app_ui_create(app_t *app, lv_disp_t *disp) {
    lv_draw_sdl_drv_param_t *param = disp->driver->user_data;
    app_ui_t *ui = calloc(1, sizeof(app_ui_t));
    ui->app = app;
    ui->window = param->user_data;
    ui->root = lv_disp_get_scr_act(disp);
    ui->fm = lv_fragment_manager_create(NULL);

    lv_group_t *group = lv_group_create();
    lv_group_set_editing(group, 0);
    lv_group_set_default(group);
    ui->group = group;

    ui->indev.mouse = app_lv_mouse_indev_init();
    ui->indev.keypad = app_indev_keypad_init();

    app_ui_fontset_set_default_size(ui, &ui->iconfont);
    app_ui_fontset_init_mem(&ui->iconfont, "bootstrap-icons", ttf_bootstrap_icons_data,
                            ttf_bootstrap_icons_size);

    lv_obj_set_style_bg_opa(ui->root, LV_OPA_0, 0);

    lv_style_init(&ui->styles.action_btn_label);
    lv_style_set_text_font(&ui->styles.action_btn_label, ui->iconfont.large);
    return ui;
}

void app_ui_created(app_ui_t *ui) {
    app_ui_push_fragment(ui, &launcher_fragment_class, NULL);
}

void app_ui_destroy(app_ui_t *ui) {
    lv_group_set_default(NULL);
    lv_group_del(ui->group);

    lv_style_reset(&ui->styles.action_btn_label);

    app_ui_fontset_deinit(&ui->iconfont);
    lv_fragment_manager_del(ui->fm);

    app_indev_keypad_deinit(ui->indev.keypad);
    app_lv_mouse_indev_deinit(ui->indev.mouse);

    free(ui);
}

void app_ui_set_ignore_keys(app_ui_t *ui, bool ignore) {
    app_indev_set_ignore_input(ui->indev.keypad, ignore);
}

void app_ui_push_fragment(app_ui_t *ui, const lv_fragment_class_t *cls, void *args) {
    app_ui_fragment_args_t fargs = {ui->app, args};
    lv_fragment_t *f = lv_fragment_create(cls, &fargs);
    lv_fragment_manager_push(ui->fm, f, &ui->root);
}

void app_ui_pop_fragment(app_ui_t *ui) {
    lv_fragment_manager_pop(ui->fm);
}

void app_ui_send_event(app_ui_t *ui, app_event_type_t type, app_ui_event_data_t *data) {
    lv_fragment_manager_send_event(ui->fm, type, data);
}
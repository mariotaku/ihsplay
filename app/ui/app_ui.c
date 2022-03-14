#include <stdlib.h>
#include <lvgl.h>

#include "app_ui.h"
#include "launcher.h"

app_ui_t *app_ui_create(app_t *app, lv_disp_t *disp) {
    app_ui_t *ui = calloc(1, sizeof(app_ui_t));
    ui->app = app;
    ui->root = lv_disp_get_scr_act(disp);
    ui->fm = lv_fragment_manager_create(NULL);

    app_ui_push_fragment(ui, &launcher_fragment_class, NULL);
    return ui;
}

void app_ui_destroy(app_ui_t *ui) {
    lv_fragment_manager_del(ui->fm);
    free(ui);
}

void app_ui_push_fragment(app_ui_t *ui, const lv_fragment_class_t *cls, void *args) {
    app_ui_fragment_args_t fargs = {ui->app, args};
    lv_fragment_t *f = lv_fragment_create(cls, &fargs);
    lv_fragment_manager_push(ui->fm, f, &ui->root);
}
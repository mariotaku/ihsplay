#include <src/draw/sw/lv_draw_sw.h>
#include <assert.h>
#include <stdio.h>
#include "lvgl.h"

#include "lv_gridview.h"

typedef struct grid_data_t {
    int size;
} grid_data_t;

static int adapter_item_count(lv_obj_t *grid, void *data);

static lv_obj_t *adapter_create_view(lv_obj_t *grid);

static void adapter_bind_view(lv_obj_t *grid, lv_obj_t *item, void *data, int index);

static const lv_gridview_adapter_t adapter = {
        .item_count = adapter_item_count,
        .create_view = adapter_create_view,
        .bind_view = adapter_bind_view,
};

void test_basic() {
    lv_obj_t *obj = lv_gridview_create(lv_scr_act());
    lv_obj_set_size(obj, LV_PCT(100), LV_PCT(100));
    grid_data_t data = {.size = 10};
    lv_gridview_set_config(obj, 5, 64, LV_GRID_ALIGN_CENTER, LV_GRID_ALIGN_CENTER);
    lv_gridview_set_adapter(obj, &adapter);
    lv_gridview_set_data_advanced(obj, &data, NULL, -1);
    lv_task_handler();
    assert(lv_obj_get_child_cnt(obj) == data.size + 1);

    for (int i = 0; i < 10; i++) {
        lv_gridview_data_change_t changes[] = {
                {.start = data.size, .add_count = 1, .remove_count = 0}
        };
        data.size++;
        lv_gridview_set_data_advanced(obj, &data, changes, 1);
        lv_task_handler();

        assert(lv_obj_get_child_cnt(obj) == data.size + 1);
    }

    lv_obj_del(obj);
}

static int adapter_item_count(lv_obj_t *grid, void *data) {
    LV_UNUSED(grid);
    return ((grid_data_t *) data)->size;
}

static lv_obj_t *adapter_create_view(lv_obj_t *grid) {
    return lv_label_create(grid);
}

static void adapter_bind_view(lv_obj_t *grid, lv_obj_t *item, void *data, int index) {
    lv_label_set_text_fmt(item, "%d", index);
}

int main() {
    lv_init();
    lv_disp_draw_buf_t *draw_buf = lv_mem_alloc(sizeof(lv_disp_draw_buf_t));
    lv_color_t *buf1 = lv_mem_alloc(sizeof(lv_color_t) * 320 * 240);
    lv_disp_draw_buf_init(draw_buf, buf1, NULL, 320 * 240);
    lv_disp_drv_t *drv = malloc(sizeof(lv_disp_drv_t));
    lv_disp_drv_init(drv);


    lv_draw_ctx_t *draw_ctx = lv_mem_alloc(sizeof(lv_draw_sw_ctx_t));
    lv_draw_sw_init_ctx(drv, draw_ctx);
    drv->draw_ctx = draw_ctx;
    drv->draw_buf = draw_buf;

    lv_disp_t *disp = lv_disp_drv_register(drv);
    lv_disp_set_default(disp);

    test_basic();

    free(draw_ctx);
    free(drv);
    free(draw_buf);
    free(buf1);
    return 0;
}

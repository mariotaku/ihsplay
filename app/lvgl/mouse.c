#include "mouse.h"

#include <SDL.h>

static void read_cb(lv_indev_drv_t *drv, lv_indev_data_t *data);

void app_lv_mouse_init() {
    lv_indev_drv_t *driver = malloc(sizeof(lv_indev_drv_t));
    lv_indev_drv_init(driver);
    driver->type = LV_INDEV_TYPE_POINTER;
    driver->read_cb = read_cb;
    lv_indev_t *indev = lv_indev_drv_register(driver);
    lv_indev_set_group(indev, lv_group_get_default());
}

static void read_cb(lv_indev_drv_t *drv, lv_indev_data_t *data) {
    int x, y;
    Uint32 buttons = SDL_GetMouseState(&x, &y);
    data->state = (buttons & SDL_BUTTON_LEFT) ? LV_INDEV_STATE_PRESSED : LV_INDEV_STATE_RELEASED;
    data->point.x = x;
    data->point.y = y;
    data->continue_reading = false;
}
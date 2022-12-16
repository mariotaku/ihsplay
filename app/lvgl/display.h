#pragma once

#include <lvgl.h>
#include <SDL.h>

lv_disp_t *app_lv_disp_init(SDL_Window *window);

lv_disp_t *app_lv_disp_deinit(lv_disp_t *disp);

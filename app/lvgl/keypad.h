#pragma once

#include <lvgl.h>
#include <SDL2/SDL.h>

lv_indev_t *app_lv_keypad_indev_init();

void app_lv_keypad_indev_deinit(lv_indev_t *indev);

void app_sdl_key_event(lv_indev_t *indev, const SDL_KeyboardEvent *event);

void app_sdl_cbutton_event(lv_indev_t *indev, const SDL_ControllerButtonEvent *event);
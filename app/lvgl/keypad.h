#pragma once

#include <lvgl.h>
#include <SDL2/SDL.h>

typedef struct app_t app_t;

lv_indev_t *app_indev_keypad_init(app_t *app);

void app_indev_keypad_deinit(lv_indev_t *indev);

void app_indev_set_ignore_input(lv_indev_t *indev, bool ignore);

void app_indev_sdl_key_event(lv_indev_t *indev, const SDL_KeyboardEvent *event);

void app_indev_sdl_cbutton_event(lv_indev_t *indev, const SDL_ControllerButtonEvent *event);
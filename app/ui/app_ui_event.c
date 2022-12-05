#include "app_ui.h"

#include "lvgl/keypad.h"

void app_ui_sdl_event(app_ui_t *ui, const SDL_Event *event) {
    switch (event->type) {
        case SDL_KEYUP:
        case SDL_KEYDOWN: {
            app_indev_sdl_key_event(ui->indev.keypad, &event->key);
            break;
        }
        case SDL_CONTROLLERBUTTONUP:
        case SDL_CONTROLLERBUTTONDOWN: {
            app_indev_sdl_cbutton_event(ui->indev.keypad, &event->cbutton);
            break;
        }
    }
}
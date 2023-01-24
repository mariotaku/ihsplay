#include "ui/app_ui.h"

#include "lvgl/keypad.h"
#include "app.h"
#include "backend/input_manager.h"

void app_sdl_input_event(app_t *app, const SDL_Event *event) {
    switch (event->type) {
        case SDL_CONTROLLERDEVICEADDED: {
            input_manager_sdl_gamepad_added(app->input_manager, event->cdevice.which);
            app_post_event(app, APP_UI_GAMEPAD_DEVICE_CHANGED, NULL, NULL);
            break;
        }
        case SDL_CONTROLLERDEVICEREMOVED: {
            input_manager_sdl_gamepad_removed(app->input_manager, event->cdevice.which);
            app_post_event(app, APP_UI_GAMEPAD_DEVICE_CHANGED, NULL, NULL);
            break;
        }
        case SDL_KEYUP:
        case SDL_KEYDOWN: {
            app_indev_keypad_sdl_key_event(app->ui->indev.keypad, &event->key);
            break;
        }
        case SDL_CONTROLLERBUTTONUP:
        case SDL_CONTROLLERBUTTONDOWN: {
            app_indev_keypad_sdl_cbutton_event(app->ui->indev.keypad, &event->cbutton);
            break;
        }
    }
}
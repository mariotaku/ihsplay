#include "mouse.h"

#include <SDL.h>

typedef struct keyboard_state_t {
    bool ignore_input;
    uint32_t key;
    bool pressed;
} keyboard_state_t;

static void read_cb(lv_indev_drv_t *drv, lv_indev_data_t *data);

static uint32_t key_from_keysym(const SDL_Keysym *keysym);

static uint32_t key_from_cbutton(uint8_t button);

lv_indev_t *app_indev_keypad_init() {
    lv_indev_drv_t *driver = malloc(sizeof(lv_indev_drv_t));
    lv_indev_drv_init(driver);
    driver->type = LV_INDEV_TYPE_KEYPAD;
    driver->read_cb = read_cb;
    driver->user_data = calloc(1, sizeof(keyboard_state_t));
    lv_indev_t *indev = lv_indev_drv_register(driver);
    lv_indev_set_group(indev, lv_group_get_default());
    return indev;
}

void app_indev_keypad_deinit(lv_indev_t *indev) {
    lv_indev_drv_t *driver = indev->driver;
    lv_indev_delete(indev);
    free(driver->user_data);
    free(driver);
}

void app_indev_set_ignore_input(lv_indev_t *indev, bool ignore) {
    keyboard_state_t *state = indev->driver->user_data;
    state->ignore_input = ignore;
}

void app_indev_sdl_key_event(lv_indev_t *indev, const SDL_KeyboardEvent *event) {
    uint32_t key = key_from_keysym(&event->keysym);
    if (key == 0) {
        return;
    }
    keyboard_state_t *state = indev->driver->user_data;
    if (event->state == SDL_PRESSED) {
        if (state->key == 0) {
            state->key = key;
            state->pressed = true;
        }
    } else if (event->state == SDL_RELEASED) {
        if (state->key == key) {
            state->key = 0;
            state->pressed = false;
        }
    }

}

void app_indev_sdl_cbutton_event(lv_indev_t *indev, const SDL_ControllerButtonEvent *event) {
    uint32_t key = key_from_cbutton(event->button);
    if (key == 0) {
        return;
    }
    keyboard_state_t *state = indev->driver->user_data;
    if (event->state == SDL_PRESSED) {
        if (state->key == 0) {
            state->key = key;
            state->pressed = true;
        }
    } else if (event->state == SDL_RELEASED) {
        if (state->key == key) {
            state->key = 0;
            state->pressed = false;
        }
    }
}

static void read_cb(lv_indev_drv_t *drv, lv_indev_data_t *data) {
    const keyboard_state_t *state = drv->user_data;
    data->key = state->key;
    if (state->ignore_input) {
        data->state = false;
    } else {
        data->state = state->pressed ? LV_INDEV_STATE_PRESSED : LV_INDEV_STATE_RELEASED;
    }
    data->continue_reading = false;
}

static uint32_t key_from_keysym(const SDL_Keysym *keysym) {
    switch (keysym->sym) {
        case SDLK_UP:
            return LV_KEY_UP;
        case SDLK_DOWN:
            return LV_KEY_DOWN;
        case SDLK_LEFT:
            return LV_KEY_LEFT;
        case SDLK_RIGHT:
            return LV_KEY_RIGHT;
        case SDLK_RETURN:
        case SDLK_RETURN2:
        case SDLK_KP_ENTER:
            return LV_KEY_ENTER;
        case SDLK_ESCAPE:
            return LV_KEY_ESC;
        case SDLK_TAB:
            return keysym->mod & KMOD_SHIFT ? LV_KEY_PREV : LV_KEY_NEXT;
        default:
#ifdef SDL_WEBOS_SCANCODE_BACK
            switch ((int) keysym->scancode) {
                case SDL_WEBOS_SCANCODE_BACK:
                    return LV_KEY_ESC;
                case SDL_WEBOS_SCANCODE_CH_UP:
                    return LV_KEY_PREV;
                case SDL_WEBOS_SCANCODE_CH_DOWN:
                    return LV_KEY_NEXT;
                default:
                    break;
            }
#endif
            return 0;
    }
}

static uint32_t key_from_cbutton(uint8_t button) {
    switch (button) {
        case SDL_CONTROLLER_BUTTON_DPAD_UP:
            return LV_KEY_UP;
        case SDL_CONTROLLER_BUTTON_DPAD_DOWN:
            return LV_KEY_DOWN;
        case SDL_CONTROLLER_BUTTON_DPAD_LEFT:
            return LV_KEY_LEFT;
        case SDL_CONTROLLER_BUTTON_DPAD_RIGHT:
            return LV_KEY_RIGHT;
        case SDL_CONTROLLER_BUTTON_A:
            return LV_KEY_ENTER;
        case SDL_CONTROLLER_BUTTON_B:
            return LV_KEY_ESC;
        case SDL_CONTROLLER_BUTTON_BACK:
            return LV_KEY_NEXT;
        default:
            return 0;
    }
}
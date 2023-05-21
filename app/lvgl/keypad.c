#include "mouse.h"
#include "logging.h"
#include "app.h"
#include "ui/app_ui.h"

#include <SDL.h>

typedef struct keyboard_state_t {
    app_t *app;
    bool ignore_input;
    /**
     * Key code of last pressed key event, will be 0 if released
     */
    uint32_t key;
    /**
     * Key code of last key event, no matter its released or not
     */
    uint32_t ev_key;
    lv_indev_state_t state;
    bool changed;
} keyboard_state_t;

static void read_cb(lv_indev_drv_t *drv, lv_indev_data_t *data);

static void handle_esc(keyboard_state_t *state);

static uint32_t key_from_keysym(const SDL_Keysym *keysym);

static uint32_t key_from_cbutton(uint8_t button);

lv_indev_t *app_indev_keypad_init(app_t *app) {
    lv_indev_drv_t *driver = malloc(sizeof(lv_indev_drv_t));
    lv_indev_drv_init(driver);
    driver->type = LV_INDEV_TYPE_KEYPAD;
    driver->read_cb = read_cb;
    keyboard_state_t *state = calloc(1, sizeof(keyboard_state_t));
    state->app = app;
    driver->user_data = state;
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

void app_indev_keypad_set_ignore(lv_indev_t *indev, bool ignore) {
    keyboard_state_t *state = indev->driver->user_data;
    state->ignore_input = ignore;
}

void app_indev_keypad_sdl_key_event(lv_indev_t *indev, const SDL_KeyboardEvent *event) {
    keyboard_state_t *state = indev->driver->user_data;
    uint32_t key = key_from_keysym(&event->keysym);
    if (key == 0) {
#ifdef SDL_WEBOS_SCANCODE_EXIT
        if (event->state == SDL_RELEASED && event->keysym.scancode == SDL_WEBOS_SCANCODE_EXIT) {
            app_post_event(state->app, APP_UI_NAV_QUIT, NULL, NULL);
        }
#endif
        return;
    }
    if (event->state == SDL_PRESSED) {
        if (state->key == 0) {
            state->key = state->ev_key = key;
            state->state = LV_INDEV_STATE_PRESSED;
            state->changed = true;
        }
    } else if (event->state == SDL_RELEASED) {
        if (state->key == key) {
            state->ev_key = key;
            state->key = 0;
            state->state = LV_INDEV_STATE_RELEASED;
            state->changed = true;
        }
    }
}

void app_indev_keypad_sdl_cbutton_event(lv_indev_t *indev, const SDL_ControllerButtonEvent *event) {
    uint32_t key = key_from_cbutton(event->button);
    if (key == 0) {
        return;
    }
    keyboard_state_t *state = indev->driver->user_data;
    if (event->state == SDL_PRESSED) {
        if (state->key == 0) {
            state->key = state->ev_key = key;
            state->state = LV_INDEV_STATE_PRESSED;
            state->changed = true;
        }
    } else if (event->state == SDL_RELEASED) {
        if (state->key == key) {
            state->ev_key = key;
            state->key = 0;
            state->state = LV_INDEV_STATE_RELEASED;
            state->changed = true;
        }
    }
}

void app_indev_keypad_inject_key(lv_indev_t *indev, lv_key_t key, bool pressed) {
    keyboard_state_t *state = indev->driver->user_data;
    if (pressed) {
        if (state->key == 0) {
            state->key = state->ev_key = key;
            state->state = LV_INDEV_STATE_PRESSED;
            state->changed = true;
        }
    } else {
        if (state->key == key) {
            state->ev_key = key;
            state->key = 0;
            state->state = LV_INDEV_STATE_RELEASED;
            state->changed = true;
        }
    }
}

static void read_cb(lv_indev_drv_t *drv, lv_indev_data_t *data) {
    keyboard_state_t *state = drv->user_data;
    if (state->changed && state->ev_key == LV_KEY_ESC) {
        handle_esc(state);
    }
    state->changed = false;
    data->key = state->key;
    if (state->ignore_input) {
        data->state = false;
    } else {
        data->state = state->state;
    }
    data->continue_reading = false;
}

/**
 * Special case for esc keys
 * @param state
 */
static void handle_esc(keyboard_state_t *state) {
    lv_group_t *group = app_ui_get_input_group(state->app->ui);
    lv_obj_t *focused = lv_group_get_focused(group);
    if (focused == NULL) {
        // Next key event will not be handled by any object. Send global BACK event
        if (state->state == LV_INDEV_STATE_RELEASED) {
            app_post_event(state->app, APP_UI_NAV_BACK, NULL, NULL);
        }
        return;
    }
    // TODO: for things like dropdown, cancel event will be sent when ESC pressed.
    //  So we can compare PRESS and RELEASE event to see if it's closed or not. Send BACK event if no state change.
    if (state->state == LV_INDEV_STATE_PRESSED) {
        // Backup object state (e.g. is dropdown opened)
    } else {
        // Compare object state, and send BACK event if no change.
        app_post_event(state->app, APP_UI_NAV_BACK, NULL, NULL);
    }
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
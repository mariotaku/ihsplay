#pragma once

#include <SDL2/SDL.h>
#include <stdbool.h>
#include "ihslib/hid/sdl.h"
#include "array_list.h"

typedef struct opened_controller_t {
    SDL_GameController *controller;
    SDL_JoystickID id;
} opened_controller_t;

typedef struct input_manager_t {
    array_list_t controllers;
    IHS_HIDProvider *hid_provider;
    bool ignore_next_mouse_movement;
} input_manager_t;

input_manager_t *input_manager_create();

void input_manager_destroy(input_manager_t *manager);

IHS_HIDProvider *input_manager_get_hid_provider(input_manager_t *manager);

void input_manager_sdl_gamepad_added(input_manager_t *manager, int which);

void input_manager_sdl_gamepad_removed(input_manager_t *manager, SDL_JoystickID which);

size_t input_manager_sdl_gamepad_count(const input_manager_t *manager);

/**
 * Tell the app to ignore next mouse movement, for manual moving the cursor position
 * @param manager
 */
void input_manager_ignore_next_mouse_movement(input_manager_t *manager);

bool input_manager_get_and_reset_mouse_movement(input_manager_t *manager);

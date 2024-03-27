#pragma once

#include "stream_manager.h"

bool stream_input_handle_key_event(stream_manager_t *manager, const SDL_KeyboardEvent *event);

bool stream_input_handle_mouse_event(stream_manager_t *manager, const SDL_Event *event);

bool stream_input_handle_touch_event(stream_manager_t *manager, const SDL_TouchFingerEvent *event);
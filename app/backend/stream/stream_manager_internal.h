#pragma once

#include "stream_manager.h"
#include "stream_media.h"

#include "util/array_list.h"

typedef enum stream_manager_state_t {
    STREAM_MANAGER_STATE_IDLE,
    STREAM_MANAGER_STATE_CONNECTING,
    STREAM_MANAGER_STATE_STREAMING,
    STREAM_MANAGER_STATE_DISCONNECTING,
} stream_manager_state_t;

struct stream_manager_t {
    app_t *app;
    array_list_t *listeners;

    stream_manager_state_t state;

    stream_media_session_t *media;
    IHS_Session *session;
    SDL_TimerID back_timer;
    int back_counter;
    bool overlay_opened;
    bool requested_disconnect;

    int viewport_width, viewport_height;
    int capture_width, capture_height;
    int overlay_height;
};

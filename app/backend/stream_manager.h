#pragma once

#include <ihslib.h>

#include <SDL_events.h>

typedef struct app_t app_t;
typedef struct host_manager_t host_manager_t;

typedef struct stream_manager_t stream_manager_t;

typedef struct stream_manager_callbacks_t {
    void (*connected)(const IHS_SessionInfo *info, void *context);

    void (*disconnected)(const IHS_SessionInfo *info, bool requested, void *context);
} stream_manager_listener_t;

stream_manager_t *stream_manager_create(app_t *app);

void stream_manager_destroy(stream_manager_t *manager);

void stream_manager_register_listener(stream_manager_t *manager, const stream_manager_listener_t *listener,
                                      void *context);

void stream_manager_unregister_listener(stream_manager_t *manager, const stream_manager_listener_t *listener);

bool stream_manager_start_session(stream_manager_t *manager, const IHS_SessionInfo *info);

IHS_Session *stream_manager_active_session(const stream_manager_t *manager);

void stream_manager_stop_active(stream_manager_t *manager);

bool stream_manager_handle_event(stream_manager_t *manager, const SDL_Event *event);

void stream_manager_set_viewport_size(stream_manager_t *manager, int width, int height);

bool stream_manager_is_overlay_opened(const stream_manager_t *manager);

void stream_manager_set_overlay_height(stream_manager_t *manager, int height);

bool stream_manager_set_overlay_opened(stream_manager_t *manager, bool opened);

void stream_manager_set_capture_size(stream_manager_t *manager, int width, int height);
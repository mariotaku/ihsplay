#include <assert.h>
#include "stream_manager.h"

#include "app.h"
#include "util/listeners_list.h"

#include "ihslib/hid/sdl.h"

#include "ss4s.h"
#include "stream_media.h"

static void session_initialized(IHS_Session *session, void *context);

static void session_finalized(IHS_Session *session, void *context);

static void session_configuring(IHS_Session *session, IHS_SessionConfig *config, void *context);

static void session_connected(IHS_Session *session, void *context);

static void session_disconnected(IHS_Session *session, void *context);

static void session_connected_main(app_t *app, void *context);

static void session_disconnected_main(app_t *app, void *context);

static void destroy_session_main(app_t *app, void *context);

static void controller_back_pressed(stream_manager_t *manager);

static void controller_back_released(stream_manager_t *manager);

static Uint32 back_timer_callback(Uint32 duration, void *param);

static bool should_intercept_event(Uint32 type);

#define BACK_COUNTER_MAX 100

typedef enum stream_manager_state_t {
    STREAM_MANAGER_STATE_IDLE,
    STREAM_MANAGER_STATE_CONNECTING,
    STREAM_MANAGER_STATE_STREAMING,
    STREAM_MANAGER_STATE_DISCONNECTING,
} stream_manager_state_t;

struct stream_manager_t {
    app_t *app;
    array_list_t *listeners;
    IHS_HIDProvider *hid_provider;

    stream_manager_state_t state;

    stream_media_session_t *media;
    IHS_Session *session;
    SDL_TimerID back_timer;
    int back_counter;
    bool overlay_opened;
    bool requested_disconnect;
};

typedef struct event_context_t {
    stream_manager_t *manager;
    void *arg1;
    uint32_t value1;
} event_context_t;

static const IHS_StreamSessionCallbacks session_callbacks = {
        .initialized = session_initialized,
        .configuring = session_configuring,
        .connected = session_connected,
        .disconnected = session_disconnected,
        .finalized = session_finalized,
};

stream_manager_t *stream_manager_create(app_t *app) {
    stream_manager_t *manager = calloc(1, sizeof(stream_manager_t));
    manager->app = app;
    manager->listeners = listeners_list_create();
    manager->hid_provider = IHS_HIDProviderSDLCreate(false);
    return manager;
}

void stream_manager_destroy(stream_manager_t *manager) {
    switch (manager->state) {
        case STREAM_MANAGER_STATE_IDLE: {
            break;
        }
        default: {
            destroy_session_main(manager->app, manager->session);
            break;
        }
    }
    listeners_list_destroy(manager->listeners);
    IHS_HIDProviderSDLDestroy(manager->hid_provider);
    free(manager);
}

void stream_manager_register_listener(stream_manager_t *manager, const stream_manager_listener_t *listener,
                                      void *context) {
    listeners_list_add(manager->listeners, listener, context);
}

void stream_manager_unregister_listener(stream_manager_t *manager, const stream_manager_listener_t *listener) {
    listeners_list_remove(manager->listeners, listener);
}

bool stream_manager_start_session(stream_manager_t *manager, const IHS_SessionInfo *info) {
    app_assert_main_thread(manager->app);
    if (manager->state != STREAM_MANAGER_STATE_IDLE) {
        return false;
    }
    manager->media = stream_media_create();
    IHS_Session *session = IHS_SessionCreate(&manager->app->client_config, info);
    IHS_SessionSetLogFunction(session, app_ihs_log);
    IHS_SessionSetSessionCallbacks(session, &session_callbacks, manager);
    IHS_SessionSetAudioCallbacks(session, stream_media_audio_callbacks(), manager->media);
    IHS_SessionSetVideoCallbacks(session, stream_media_video_callbacks(), manager->media);
    IHS_SessionHIDAddProvider(session, manager->hid_provider);
    manager->state = STREAM_MANAGER_STATE_CONNECTING;
    app_ihs_log(IHS_LogLevelInfo, "StreamManager", "Change state to CONNECTING");
    manager->session = session;
    manager->back_counter = 0;
    manager->back_timer = 0;

    IHS_SessionConnect(session);
    return true;
}

IHS_Session *stream_manager_active_session(const stream_manager_t *manager) {
    if (manager->state != STREAM_MANAGER_STATE_STREAMING) {
        return NULL;
    }
    return manager->session;
}

void stream_manager_stop_active(stream_manager_t *manager) {
    if (manager->state != STREAM_MANAGER_STATE_STREAMING) {
        return;
    }
    manager->requested_disconnect = true;
    IHS_SessionDisconnect(manager->session);
}

bool stream_manager_handle_event(stream_manager_t *manager, const SDL_Event *event) {
    if (manager->state != STREAM_MANAGER_STATE_STREAMING) {
        return false;
    }
    switch (event->type) {
        case SDL_CONTROLLERBUTTONDOWN: {
            if (event->cbutton.button == SDL_CONTROLLER_BUTTON_BACK) {
                controller_back_pressed(manager);
            }
            break;
        }
        case SDL_CONTROLLERBUTTONUP: {
            if (event->cbutton.button == SDL_CONTROLLER_BUTTON_BACK) {
                controller_back_released(manager);
            }
            break;
        }
    }
    if (manager->overlay_opened && should_intercept_event(event->type)) {
        return true;
    }
    return IHS_HIDHandleSDLEvent(manager->session, event);
}

bool stream_manager_is_overlay_opened(const stream_manager_t *manager) {
    if (manager->state != STREAM_MANAGER_STATE_STREAMING) {
        return false;
    }
    return manager->overlay_opened;
}

bool stream_manager_set_overlay_opened(stream_manager_t *manager, bool opened) {
    if (manager->state != STREAM_MANAGER_STATE_STREAMING) {
        return false;
    }
    manager->overlay_opened = opened;
    stream_media_set_overlay_shown(manager->media, opened);
    if (opened) {
        app_post_event(manager->app, APP_UI_REQUEST_OVERLAY, NULL, NULL);
    } else {
        app_post_event(manager->app, APP_UI_CLOSE_OVERLAY, NULL, NULL);
    }
    return true;
}

static void session_initialized(IHS_Session *session, void *context) {
    stream_manager_t *manager = (stream_manager_t *) context;
    assert (manager->state == STREAM_MANAGER_STATE_CONNECTING);
    IHS_SessionConnect(session);
}

static void session_finalized(IHS_Session *session, void *context) {
    stream_manager_t *manager = (stream_manager_t *) context;
    assert(manager->state == STREAM_MANAGER_STATE_DISCONNECTING);
    assert(manager->session == session);
    manager->state = STREAM_MANAGER_STATE_IDLE;
    app_ihs_log(IHS_LogLevelInfo, "StreamManager", "Change state to IDLE");
    app_run_on_main(manager->app, destroy_session_main, session);
}

static void session_configuring(IHS_Session *session, IHS_SessionConfig *config, void *context) {
    (void) session;
    (void) context;
    config->enableHevc = false;
}

static void session_connected(IHS_Session *session, void *context) {
    stream_manager_t *manager = (stream_manager_t *) context;
    assert(manager->state == STREAM_MANAGER_STATE_CONNECTING);
    assert(manager->session == session);
    manager->state = STREAM_MANAGER_STATE_STREAMING;
    app_ihs_log(IHS_LogLevelInfo, "StreamManager", "Change state to STREAMING");
    event_context_t ec = {
            .manager = manager,
            .arg1 = (void *) IHS_SessionGetInfo(session),
    };
    app_run_on_main_sync(manager->app, session_connected_main, &ec);
    IHS_SessionHIDNotifyDeviceChange(session);
}

static void session_disconnected(IHS_Session *session, void *context) {
    stream_manager_t *manager = (stream_manager_t *) context;
    assert(manager->state == STREAM_MANAGER_STATE_STREAMING);
    assert(manager->session == session);
    if (manager->back_timer != 0) {
        SDL_RemoveTimer(manager->back_timer);
        manager->back_timer = 0;
    }
    bool requested = manager->requested_disconnect;
    manager->state = STREAM_MANAGER_STATE_DISCONNECTING;
    app_ihs_log(IHS_LogLevelInfo, "StreamManager", "Change state to DISCONNECTING");
    event_context_t ec = {
            .manager = manager,
            .arg1 = (void *) IHS_SessionGetInfo(session),
            .value1 = requested
    };
    app_run_on_main_sync(manager->app, session_disconnected_main, &ec);
}

static void session_connected_main(app_t *app, void *context) {
    (void) app;
    event_context_t *ec = context;
    stream_manager_t *manager = ec->manager;
    listeners_list_notify(manager->listeners, stream_manager_listener_t, connected, (const IHS_SessionInfo *) ec->arg1);
}

static void session_disconnected_main(app_t *app, void *context) {
    (void) app;
    event_context_t *ec = context;
    stream_manager_t *manager = ec->manager;
    listeners_list_notify(manager->listeners, stream_manager_listener_t, disconnected,
                          (const IHS_SessionInfo *) ec->arg1, ec->value1);
}

static void destroy_session_main(app_t *app, void *context) {
    (void) app;
    IHS_Session *session = context;
    IHS_SessionThreadedJoin(session);
    IHS_SessionDestroy(session);
    stream_media_destroy(app->stream_manager->media);
    app->stream_manager->media = NULL;
}

static void controller_back_pressed(stream_manager_t *manager) {
    if (manager->state != STREAM_MANAGER_STATE_STREAMING || manager->back_timer != 0) {
        return;
    }
    manager->back_counter = 0;
    manager->back_timer = SDL_AddTimer(300, back_timer_callback, manager);
}

static void controller_back_released(stream_manager_t *manager) {
    if (manager->state != STREAM_MANAGER_STATE_STREAMING || manager->back_timer == 0) {
        return;
    }
    SDL_RemoveTimer(manager->back_timer);
    manager->back_timer = 0;
}

static bool should_intercept_event(Uint32 type) {
    switch (type) {
        case SDL_CONTROLLERDEVICEADDED:
        case SDL_CONTROLLERDEVICEREMOVED:
        case SDL_CONTROLLERDEVICEREMAPPED:
            return false;
        default:
            return true;
    }
}

static Uint32 back_timer_callback(Uint32 duration, void *param) {
    (void) duration;
    stream_manager_t *manager = (stream_manager_t *) param;
    if (manager->state != STREAM_MANAGER_STATE_STREAMING) {
        return 0;
    }
    manager->back_counter += 1;
    if (manager->back_counter >= BACK_COUNTER_MAX) {
        manager->back_timer = 0;
        manager->back_counter = 0;
        IHS_HIDResetSDLGameControllers(manager->session);
        app_ihs_logf(IHS_LogLevelInfo, "Streaming", "Requesting overlay");
        stream_manager_set_overlay_opened(manager, true);
        return 0;
    }
    return 16;
}


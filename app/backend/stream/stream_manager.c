#include <assert.h>
#include "stream_manager.h"

#include "app.h"
#include "ui/app_ui.h"
#include "util/listeners_list.h"

#include "ihslib/hid/sdl.h"

#include "ss4s.h"

#include "stream_media.h"
#include "stream_input.h"

#include "backend/input_manager.h"
#include "logging/app_logging.h"

static void session_initialized(IHS_Session *session, void *context);

static void session_finalized(IHS_Session *session, void *context);

static void session_configuring(IHS_Session *session, IHS_SessionConfig *config, void *context);

static void session_connected(IHS_Session *session, void *context);

static void session_disconnected(IHS_Session *session, void *context);

static void session_show_cursor(IHS_Session *session, float x, float y, void *context);

// Main thread callbacks

static void session_connected_main(app_t *app, void *context);

static void session_disconnected_main(app_t *app, void *context);

static void session_show_cursor_main(app_t *app, void *context);

static void destroy_session_main(app_t *app, void *context);

static void controller_back_pressed(stream_manager_t *manager);

static void controller_back_released(stream_manager_t *manager);

static Uint32 back_timer_callback(Uint32 duration, void *param);

static void back_timer_progress_main(app_t *app, void *context);

static void back_timer_finish_main(app_t *app, void *context);

static void grab_mouse(stream_manager_t *manager, bool grab);

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

static const IHS_StreamInputCallbacks input_callbacks = {
        .showCursor = session_show_cursor,
//        .hideCursor = session_hide_cursor,
//        .setCursor = session_set_cursor,
//        .cursorImage = session_cursor_image,
};

stream_manager_t *stream_manager_create(app_t *app) {
    assert(app->input_manager != NULL);
    stream_manager_t *manager = calloc(1, sizeof(stream_manager_t));
    manager->app = app;
    manager->listeners = listeners_list_create();
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
    stream_media_session_t *media = stream_media_create(manager);
    manager->media = media;
    IHS_Session *session = IHS_SessionCreate(&manager->app->client_config, info);
    IHS_SessionSetLogFunction(session, app_ihs_log);
    IHS_SessionSetSessionCallbacks(session, &session_callbacks, manager);
    IHS_SessionSetInputCallbacks(session, &input_callbacks, manager);
    IHS_SessionSetAudioCallbacks(session, stream_media_audio_callbacks(), media);
    IHS_SessionSetVideoCallbacks(session, stream_media_video_callbacks(), media);
    IHS_SessionHIDAddProvider(session, input_manager_get_hid_provider(manager->app->input_manager));
    manager->state = STREAM_MANAGER_STATE_CONNECTING;
    app_log_info("StreamManager", "Change state to CONNECTING");
    manager->session = session;
    manager->back_counter = 0;
    manager->back_timer = 0;
    manager->overlay_opened = false;

    stream_media_set_viewport_size(media, manager->viewport_width, manager->viewport_height);
    stream_media_set_overlay_height(media, manager->overlay_height);

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

bool stream_manager_intercept_event(const stream_manager_t *manager, const SDL_Event *event) {
    if (manager->state != STREAM_MANAGER_STATE_STREAMING) {
        return false;
    }
    if (manager->overlay_opened) {
        return false;
    }
    switch (event->type) {
        // Following events MUST be handled by the app too
        case SDL_CONTROLLERDEVICEADDED:
        case SDL_CONTROLLERDEVICEREMOVED:
        case SDL_CONTROLLERDEVICEREMAPPED:
            return false;
    }
    return true;
}

void stream_manager_handle_event(stream_manager_t *manager, const SDL_Event *event) {
    if (manager->state != STREAM_MANAGER_STATE_STREAMING) {
        return;
    }
    if (manager->overlay_opened) {
        switch (event->type) {
            // Following events be always handled by IHS
            case SDL_CONTROLLERDEVICEADDED:
            case SDL_CONTROLLERDEVICEREMOVED:
            case SDL_CONTROLLERDEVICEREMAPPED:
                break;
            default:
                return;
        }
    }
    switch (event->type) {
        case SDL_KEYDOWN:
        case SDL_KEYUP: {
            stream_input_handle_key_event(manager, &event->key);
            break;
        }
        case SDL_MOUSEMOTION: {
            if (input_manager_get_and_reset_mouse_movement(manager->app->input_manager)) {
                break;
            }
            if (manager->app->settings->relmouse) {
                IHS_SessionSendMouseMovement(manager->session, event->motion.xrel, event->motion.yrel);
            } else {
                int w, h;
                SDL_GetWindowSize(manager->app->ui->window, &w, &h);
                IHS_SessionSendMousePosition(manager->session, (float) event->motion.x / (float) w,
                                             (float) event->motion.y / (float) h);
            }
            break;
        }
        case SDL_MOUSEBUTTONDOWN:
        case SDL_MOUSEBUTTONUP: {
            IHS_StreamInputMouseButton button = 0;
            switch (event->button.button) {
                case SDL_BUTTON_LEFT:
                    button = IHS_MOUSE_BUTTON_LEFT;
                    break;
                case SDL_BUTTON_RIGHT:
                    button = IHS_MOUSE_BUTTON_RIGHT;
                    break;
                case SDL_BUTTON_MIDDLE:
                    button = IHS_MOUSE_BUTTON_MIDDLE;
                    break;
                case SDL_BUTTON_X1:
                    button = IHS_MOUSE_BUTTON_X1;
                    break;
                case SDL_BUTTON_X2:
                    button = IHS_MOUSE_BUTTON_X2;
                    break;
            }
            if (button != 0) {
                if (event->button.state == SDL_RELEASED) {
                    IHS_SessionSendMouseUp(manager->session, button);
                } else {
                    IHS_SessionSendMouseDown(manager->session, button);
                }
            }
            break;
        }
        case SDL_MOUSEWHEEL: {
            Sint32 x = event->wheel.x, y = event->wheel.y;
            if (event->wheel.direction == SDL_MOUSEWHEEL_FLIPPED) {
                x *= -1;
                y *= -1;
            }
            if (x != 0) {
                IHS_SessionSendMouseWheel(manager->session, x < 0 ? IHS_MOUSE_WHEEL_LEFT : IHS_MOUSE_WHEEL_RIGHT);
            }
            if (y != 0) {
                IHS_SessionSendMouseWheel(manager->session, y > 0 ? IHS_MOUSE_WHEEL_UP : IHS_MOUSE_WHEEL_DOWN);
            }
            break;
        }
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
    IHS_HIDHandleSDLEvent(manager->session, event);
}

void stream_manager_set_viewport_size(stream_manager_t *manager, int width, int height) {
    manager->viewport_width = width;
    manager->viewport_height = height;
    if (manager->media != NULL) {
        stream_media_set_viewport_size(manager->media, width, height);
    }
}

bool stream_manager_is_overlay_opened(const stream_manager_t *manager) {
    if (manager->state != STREAM_MANAGER_STATE_STREAMING) {
        return false;
    }
    return manager->overlay_opened;
}

void stream_manager_set_overlay_height(stream_manager_t *manager, int height) {
    manager->overlay_height = height;
    if (manager->media != NULL) {
        stream_media_set_overlay_height(manager->media, height);
    }
}

bool stream_manager_set_overlay_opened(stream_manager_t *manager, bool opened) {
    app_assert_main_thread(manager->app);
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
    grab_mouse(manager, !opened);
    return true;
}

void stream_manager_set_capture_size(stream_manager_t *manager, int width, int height) {
    manager->capture_width = width;
    manager->capture_height = height;
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
    app_log_info("StreamManager", "Change state to IDLE");
    app_run_on_main(manager->app, destroy_session_main, session);
}

static void session_configuring(IHS_Session *session, IHS_SessionConfig *config, void *context) {
    (void) session;
    stream_manager_t *manager = (stream_manager_t *) context;
    assert (manager->media != NULL);
    config->enableHevc = stream_media_supports_hevc(manager->media);
}

static void session_connected(IHS_Session *session, void *context) {
    stream_manager_t *manager = (stream_manager_t *) context;
    assert(manager->state == STREAM_MANAGER_STATE_CONNECTING);
    assert(manager->session == session);
    manager->state = STREAM_MANAGER_STATE_STREAMING;
    app_log_info("StreamManager", "Change state to STREAMING");
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
    app_log_info("StreamManager", "Change state to DISCONNECTING");
    event_context_t ec = {
            .manager = manager,
            .arg1 = (void *) IHS_SessionGetInfo(session),
            .value1 = requested
    };
    app_run_on_main_sync(manager->app, session_disconnected_main, &ec);
}

static void session_show_cursor(IHS_Session *session, float x, float y, void *context) {
    (void) session;
    stream_manager_t *manager = (stream_manager_t *) context;
    if (manager->capture_width <= 0 && manager->capture_height <= 0 || manager->viewport_width <= 0 ||
        manager->viewport_height <= 0) {
        return;
    }
    if (stream_manager_is_overlay_opened(manager)) {
        return;
    }
    float scale = SDL_min((float) manager->viewport_width / manager->capture_width,
                          (float) manager->viewport_height / manager->capture_height);
    float dst_width = (float) manager->capture_width * scale, dst_height = (float) manager->capture_height * scale;
    SDL_Point *point = calloc(1, sizeof(SDL_Point));
    point->x = (int) (((float) manager->viewport_width - dst_width) / 2.0f + dst_width * x);
    point->y = (int) (((float) manager->viewport_height - dst_height) / 2.0f + dst_height * y);
    app_run_on_main(manager->app, session_show_cursor_main, point);
}

static void session_connected_main(app_t *app, void *context) {
    (void) app;
    event_context_t *ec = context;
    stream_manager_t *manager = ec->manager;
    listeners_list_notify(manager->listeners, stream_manager_listener_t, connected, (const IHS_SessionInfo *) ec->arg1);
    grab_mouse(manager, true);
}

static void session_disconnected_main(app_t *app, void *context) {
    (void) app;
    event_context_t *ec = context;
    stream_manager_t *manager = ec->manager;
    grab_mouse(manager, false);
    listeners_list_notify(manager->listeners, stream_manager_listener_t, disconnected,
                          (const IHS_SessionInfo *) ec->arg1, ec->value1);
}

static void session_show_cursor_main(app_t *app, void *context) {
    stream_manager_t *manager = app->stream_manager;
    SDL_Point *point = context;
    input_manager_ignore_next_mouse_movement(manager->app->input_manager);
//    SDL_WarpMouseInWindow(app->ui->window, point->x, point->y);
    free(context);
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
    app_log_info("Streaming", "Overlay timer cancelled");
    // This function is expected to be called in main thread.
    app_assert_main_thread(manager->app);
    listeners_list_notify(manager->listeners, stream_manager_listener_t, overlay_progress_finished, false);
}

static void grab_mouse(stream_manager_t *manager, bool grab) {
#if IHSPLAY_FEATURE_RELMOUSE
    if (manager->state != STREAM_MANAGER_STATE_STREAMING) {
        SDL_SetRelativeMouseMode(SDL_FALSE);
        return;
    }
    SDL_SetRelativeMouseMode(grab ? SDL_TRUE : SDL_FALSE);
#endif
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
        app_log_info("Streaming", "Requesting overlay");
        app_run_on_main(manager->app, back_timer_finish_main, manager);
        return 0;
    } else {
        app_run_on_main(manager->app, back_timer_progress_main, manager);
    }
    return 16;
}

static void back_timer_progress_main(app_t *app, void *context) {
    (void) app;
    stream_manager_t *manager = (stream_manager_t *) context;
    listeners_list_notify(manager->listeners, stream_manager_listener_t, overlay_progress, manager->back_counter);
}

static void back_timer_finish_main(app_t *app, void *context) {
    (void) app;
    stream_manager_t *manager = (stream_manager_t *) context;
    stream_manager_set_overlay_opened(manager, true);
    listeners_list_notify(manager->listeners, stream_manager_listener_t, overlay_progress_finished, true);
}
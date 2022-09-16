#include "stream_manager.h"

#include "app.h"
#include "module.h"
#include "host_manager.h"
#include "util/listeners_list.h"

static void session_started(const IHS_SessionInfo *info, void *context);

static void session_initialized(IHS_Session *session, void *context);

static void session_finalized(IHS_Session *session, void *context);

static void session_configuring(IHS_Session *session, IHS_SessionConfig *config, void *context);

static void session_connected(IHS_Session *session, void *context);

static void session_disconnected(IHS_Session *session, void *context);

static void session_connected_main(app_t *app, void *context);

static void session_disconnected_main(app_t *app, void *context);

static void session_log(IHS_LogLevel level, const char *tag, const char *message);

typedef enum stream_manager_state_t {
    STREAM_MANAGER_STATE_IDLE,
    STREAM_MANAGER_STATE_REQUESTING,
    STREAM_MANAGER_STATE_STARTED,
} stream_manager_state_t;

struct stream_manager_t {
    app_t *app;
    host_manager_t *host_manager;
    array_list_t *listeners;
    union {
        stream_manager_state_t code;
        struct {
            stream_manager_state_t code;
            IHS_HostInfo host;
        } requesting;
        struct {
            stream_manager_state_t code;
            IHS_Session *session;
        } streaming;
    } state;
};

static const IHS_StreamSessionCallbacks session_callbacks = {
        .initialized = session_initialized,
        .configuring = session_configuring,
        .connected = session_connected,
        .disconnected = session_disconnected,
        .finalized = session_finalized,
};

static const host_manager_listener_t host_manager_listener = {
        .session_started = session_started,
};

stream_manager_t *stream_manager_create(app_t *app, host_manager_t *host_manager) {
    stream_manager_t *manager = calloc(1, sizeof(stream_manager_t));
    manager->app = app;
    manager->host_manager = host_manager;
    manager->listeners = listeners_list_create();
    host_manager_register_listener(host_manager, &host_manager_listener, manager);
    return manager;
}

void stream_manager_destroy(stream_manager_t *manager) {
    host_manager_unregister_listener(manager->host_manager, &host_manager_listener);
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

void stream_manager_start(stream_manager_t *manager, const IHS_HostInfo *host) {
    if (manager->state.code != STREAM_MANAGER_STATE_IDLE) {
        return;
    }
    manager->state.code = STREAM_MANAGER_STATE_REQUESTING;
    manager->state.requesting.host = *host;
    host_manager_request_session(manager->host_manager, host);
}

IHS_Session *stream_manager_active_session(const stream_manager_t *manager) {
    if (manager->state.code != STREAM_MANAGER_STATE_STARTED) {
        return NULL;
    }
    return manager->state.streaming.session;
}

void stream_manager_stop_active(stream_manager_t *manager) {
    if (manager->state.code != STREAM_MANAGER_STATE_STARTED) {
        return;
    }
    IHS_SessionDisconnect(manager->state.streaming.session);
}

static void session_started(const IHS_SessionInfo *info, void *context) {
    stream_manager_t *manager = (stream_manager_t *) context;
    if (manager->state.code != STREAM_MANAGER_STATE_REQUESTING) {
        return;
    }
    if (IHS_IPAddressCompare(&manager->state.requesting.host.address.ip, &info->address.ip) != 0) {
        return;
    }
    IHS_Session *session = IHS_SessionCreate(&manager->app->client_config, info);
    manager->state.code = STREAM_MANAGER_STATE_STARTED;
    manager->state.streaming.session = session;
    IHS_SessionSetLogFunction(session, session_log);
    IHS_SessionSetSessionCallbacks(session, &session_callbacks, manager);
    IHS_SessionSetAudioCallbacks(session, module_audio_callbacks(), NULL);
    IHS_SessionSetVideoCallbacks(session, module_video_callbacks(), NULL);
    IHS_SessionThreadedRun(session);
}

static void session_initialized(IHS_Session *session, void *context) {
    (void) context;
    IHS_SessionConnect(session);
}

static void session_finalized(IHS_Session *session, void *context) {

}

static void session_configuring(IHS_Session *session, IHS_SessionConfig *config, void *context) {
    config->enableHevc = false;
}

static void session_connected(IHS_Session *session, void *context) {
}

static void session_disconnected(IHS_Session *session, void *context) {
}

static void session_log(IHS_LogLevel level, const char *tag, const char *message) {
    switch (level) {
        case IHS_LogLevelInfo:
            fprintf(stderr, "[IHS.%s]\x1b[36m %s\x1b[0m\n", tag, message);
            break;
        case IHS_LogLevelWarn:
            fprintf(stderr, "[IHS.%s]\x1b[33m %s\x1b[0m\n", tag, message);
            break;
        case IHS_LogLevelError:
            fprintf(stderr, "[IHS.%s]\x1b[31m %s\x1b[0m\n", tag, message);
            break;
        case IHS_LogLevelFatal:
            fprintf(stderr, "[IHS.%s]\x1b[41m %s\x1b[0m\n", tag, message);
            break;
        default:
            fprintf(stderr, "[IHS.%s] %s\n", tag, message);
            break;
    }
}
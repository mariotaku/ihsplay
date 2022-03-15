#include <module.h>
#include "session.h"
#include "ihslib.h"
#include "app_ui.h"
#include "app.h"

typedef struct session_fragment_t {
    lv_fragment_t base;
    app_t *app;
    IHS_Session *session;
} session_fragment_t;

static void constructor(lv_fragment_t *self, void *args);

static void destructor(lv_fragment_t *self);

static lv_obj_t *create_obj(lv_fragment_t *self, lv_obj_t *container);

const lv_fragment_class_t session_fragment_class = {
        .constructor_cb = constructor,
        .destructor_cb = destructor,
        .create_obj_cb = create_obj,
        .instance_size = sizeof(session_fragment_t)
};

static void session_initialized(IHS_Session *session, void *context);

static void session_disconnected(IHS_Session *session, void *context);

static void session_disconnected_main(app_t *app, void *context);

static const IHS_StreamSessionCallbacks session_callbacks = {
        .initialized = session_initialized,
        .disconnected = session_disconnected,
};

static void constructor(lv_fragment_t *self, void *args) {
    session_fragment_t *fragment = (session_fragment_t *) self;
    const app_ui_fragment_args_t *fargs = args;
    fragment->app = fargs->app;
    fragment->session = IHS_SessionCreate(&fragment->app->client_config, fargs->data);
    IHS_SessionSetSessionCallbacks(fragment->session, &session_callbacks, fragment);
    IHS_SessionSetAudioCallbacks(fragment->session, module_audio_callbacks(), NULL);
    IHS_SessionSetVideoCallbacks(fragment->session, module_video_callbacks(), NULL);
    IHS_SessionThreadedRun(fragment->session);
}

static void destructor(lv_fragment_t *self) {
    session_fragment_t *fragment = (session_fragment_t *) self;
    IHS_SessionThreadedJoin(fragment->session);
    IHS_SessionDestroy(fragment->session);
}

static lv_obj_t *create_obj(lv_fragment_t *self, lv_obj_t *container) {
    return lv_obj_create(container);
}

static void session_initialized(IHS_Session *session, void *context) {
    LV_UNUSED(context);
    IHS_SessionConnect(session);
}

static void session_disconnected(IHS_Session *session, void *context) {
    session_fragment_t *fragment = context;
    app_run_on_main(fragment->app, session_disconnected_main, context);
}

static void session_disconnected_main(app_t *app, void *context) {
    LV_UNUSED(context);
    app_ui_pop_fragment(app->ui);
}
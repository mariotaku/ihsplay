#include "connection_fragment.h"

#include "backend/host_manager.h"

#include "lvgl/theme.h"
#include "ui/app_ui.h"
#include "ui/common/error_messages.h"
#include "ui/session/session.h"
#include "util/random.h"
#include "pin_fragment.h"


typedef struct connection_fragment_t {
    lv_fragment_t base;
    app_t *app;
    IHS_HostInfo host;
    lv_obj_t *content;
    lv_obj_t *title;
} connection_fragment_t;

static void conn_ctor(lv_fragment_t *self, void *arg);

static lv_obj_t *conn_create_obj(lv_fragment_t *self, lv_obj_t *container);

static void conn_obj_created(lv_fragment_t *self, lv_obj_t *obj);

static void conn_obj_will_del(lv_fragment_t *self, lv_obj_t *obj);

static void session_started(const IHS_HostInfo *host, const IHS_SessionInfo *info, void *context);

static void session_start_failed(const IHS_HostInfo *host, IHS_StreamingResult result, void *context);

static void authorized(const IHS_HostInfo *host, uint64_t steam_id, void *context);

static void authorization_failed(const IHS_HostInfo *host, IHS_AuthorizationResult result, void *context);

static void open_authorization(connection_fragment_t *fragment, const IHS_HostInfo *info);

const lv_fragment_class_t connection_fragment_class = {
        .constructor_cb = conn_ctor,
        .create_obj_cb = conn_create_obj,
        .obj_created_cb = conn_obj_created,
        .obj_will_delete_cb = conn_obj_will_del,
        .instance_size = sizeof(connection_fragment_t)
};

static const host_manager_listener_t conn_host_listener = {
        .session_started = session_started,
        .session_start_failed = session_start_failed,
        .authorized = authorized,
        .authorization_failed = authorization_failed,
};

static void conn_ctor(lv_fragment_t *self, void *arg) {
    connection_fragment_t *fragment = (connection_fragment_t *) self;
    app_ui_fragment_args_t *args = arg;
    fragment->app = args->app;
    fragment->host = (*(IHS_HostInfo *) args->data);
    free(args->data);
}

static lv_obj_t *conn_create_obj(lv_fragment_t *self, lv_obj_t *container) {
    connection_fragment_t *fragment = (connection_fragment_t *) self;
    lv_obj_t *win = app_lv_win_create(container);
    fragment->title = lv_win_add_title(win, "Connecting");
    fragment->content = lv_win_get_content(win);
    return win;
}

static void conn_obj_created(lv_fragment_t *self, lv_obj_t *obj) {
    connection_fragment_t *fragment = (connection_fragment_t *) self;
    host_manager_t *hosts_manager = fragment->app->host_manager;
    host_manager_register_listener(hosts_manager, &conn_host_listener, fragment);
    host_manager_session_request(hosts_manager, &fragment->host);
}

static void conn_obj_will_del(lv_fragment_t *self, lv_obj_t *obj) {
    connection_fragment_t *fragment = (connection_fragment_t *) self;
    host_manager_t *hosts_manager = fragment->app->host_manager;
    host_manager_unregister_listener(hosts_manager, &conn_host_listener);
}

void connection_fragment_set_title(lv_fragment_t *self, const char *title) {
    connection_fragment_t *fragment = (connection_fragment_t *) self;
    lv_label_set_text(fragment->title, title);
}

static void session_started(const IHS_HostInfo *host, const IHS_SessionInfo *info, void *context) {
    connection_fragment_t *fragment = (connection_fragment_t *) context;
    session_fragment_args_t args = {
            .host = *host,
            .session = *info,
    };
    app_ui_push_fragment(fragment->app->ui, &session_fragment_class, &args);
    app_ui_remove_fragment(fragment->app->ui, (lv_fragment_t *) fragment);
}

static void session_start_failed(const IHS_HostInfo *host, IHS_StreamingResult result, void *context) {
    connection_fragment_t *fragment = (connection_fragment_t *) context;
    if (result == IHS_StreamingUnauthorized) {
        open_authorization(fragment, host);
    } else {
        const char *message = streaming_result_str(result);
        // TODO Show streaming error
    }
}

static void authorized(const IHS_HostInfo *host, uint64_t steam_id, void *context) {
    (void) host;
    (void) steam_id;
    connection_fragment_t *fragment = (connection_fragment_t *) context;
    // TODO Hide authorization UI
    // TODO Performance test?
}

static void authorization_failed(const IHS_HostInfo *host, IHS_AuthorizationResult result, void *context) {
    (void) host;
    connection_fragment_t *fragment = (connection_fragment_t *) context;
    const char *message = authorization_result_str(result);
    // TODO Show authorization error
}

static void open_authorization(connection_fragment_t *fragment, const IHS_HostInfo *info) {
    char pin[8];
    random_pin(pin);
    host_manager_authorization_request(fragment->app->host_manager, info, pin);
    lv_fragment_t *pin_fragment = app_ui_create_fragment(fragment->app->ui, &pin_fragment_class, pin);
    lv_fragment_manager_replace(fragment->base.child_manager, pin_fragment, &fragment->content);
    lv_obj_set_size(pin_fragment->obj, LV_PCT(100), LV_PCT(100));
}


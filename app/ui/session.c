#include "session.h"
#include "ihslib.h"
#include "app_ui.h"

typedef struct session_fragment_t {
    lv_fragment_t base;
    app_t *app;
    IHS_SessionConfig sess_config;
} session_fragment_t;

static void constructor(lv_fragment_t *self, void *args);

const lv_fragment_class_t session_fragment_class = {
        .constructor_cb = constructor,
        .instance_size = sizeof(session_fragment_t)
};

static void constructor(lv_fragment_t *self, void *args) {
    session_fragment_t *fragment = (session_fragment_t *) self;
    const app_ui_fragment_args_t *fargs = args;
    fragment->app = fargs->app;
    fragment->sess_config = *((IHS_SessionConfig *) fargs->data);
}
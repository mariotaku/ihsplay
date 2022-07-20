#include <module.h>
#include "session.h"
#include "ihslib.h"
#include "app_ui.h"
#include "app.h"

typedef struct session_fragment_t {
    lv_fragment_t base;
    app_t *app;
    IHS_Session *session;
    array_list_t *cursors;
} session_fragment_t;

typedef struct cursor_t {
    uint64_t id;
    SDL_Cursor *cursor;
} cursor_t;

static void constructor(lv_fragment_t *self, void *args);

static void destructor(lv_fragment_t *self);

static lv_obj_t *create_obj(lv_fragment_t *self, lv_obj_t *container);

static void session_log(IHS_LogLevel level, const char *message);

const lv_fragment_class_t session_fragment_class = {
        .constructor_cb = constructor,
        .destructor_cb = destructor,
        .create_obj_cb = create_obj,
        .instance_size = sizeof(session_fragment_t)
};

static void session_initialized(IHS_Session *session, void *context);

static void session_disconnected(IHS_Session *session, void *context);

static void session_disconnected_main(app_t *app, void *context);

static void session_show_cursor(IHS_Session *session, float x, float y, void *context);

static bool session_set_cursor(IHS_Session *session, uint64_t cursorId, void *context);

static void session_cursor_image(IHS_Session *session, const IHS_StreamInputCursorImage *image, void *context);

static const IHS_StreamSessionCallbacks session_callbacks = {
        .initialized = session_initialized,
        .disconnected = session_disconnected,
};
static const IHS_StreamInputCallbacks input_callbacks = {
        .showCursor = session_show_cursor,
        .setCursor = session_set_cursor,
        .cursorImage = session_cursor_image,
};

static void constructor(lv_fragment_t *self, void *args) {
    session_fragment_t *fragment = (session_fragment_t *) self;
    const app_ui_fragment_args_t *fargs = args;
    IHS_Session *session = IHS_SessionCreate(&fargs->app->client_config, fargs->data);
    fragment->app = fargs->app;
    fragment->session = session;
    fragment->cursors = array_list_create(sizeof(cursor_t), 16);
    IHS_SessionSetLogFunction(session, session_log);
    IHS_SessionSetSessionCallbacks(session, &session_callbacks, fragment);
    IHS_SessionSetInputCallbacks(session, &input_callbacks, fragment);
    IHS_SessionSetAudioCallbacks(session, module_audio_callbacks(), NULL);
    IHS_SessionSetVideoCallbacks(session, module_video_callbacks(), NULL);
    IHS_SessionThreadedRun(session);
}

static void destructor(lv_fragment_t *self) {
    session_fragment_t *fragment = (session_fragment_t *) self;
    IHS_SessionThreadedJoin(fragment->session);
    IHS_SessionDestroy(fragment->session);
    array_list_destroy(fragment->cursors);
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

static void session_show_cursor(IHS_Session *session, float x, float y, void *context) {
    session_fragment_t *fragment = context;
    int w = 0, h = 0;
    SDL_GetWindowSize(fragment->app->ui->window, &w, &h);
    SDL_WarpMouseInWindow(fragment->app->ui->window, (int) (x * w), (int) (y * h));
}

static bool session_set_cursor(IHS_Session *session, uint64_t cursorId, void *context) {
    session_fragment_t *fragment = context;
    cursor_t *cursor = NULL;
    for (int i = 0, j = array_list_size(fragment->cursors); i < j; ++i) {
        cursor_t *item = array_list_get(fragment->cursors, i);
        if (item->id == cursorId) {
            cursor = item;
            break;
        }
    }
    if (!cursor) {
        return false;
    }
    SDL_SetCursor(cursor->cursor);
    return true;
}

static void session_cursor_image(IHS_Session *session, const IHS_StreamInputCursorImage *image, void *context) {
    session_fragment_t *fragment = context;
    cursor_t *cursor = NULL;
    for (int i = 0, j = array_list_size(fragment->cursors); i < j; ++i) {
        cursor_t *item = array_list_get(fragment->cursors, i);
        if (item->id == image->width) {
            cursor = item;
            break;
        }
    }
    if (cursor == NULL) {
        cursor = array_list_add(fragment->cursors, -1);
    } else {
        SDL_FreeCursor(cursor->cursor);
    }
    SDL_Surface *surface = SDL_CreateRGBSurfaceFrom((void *) image->image, image->width, image->height, 32,
                                                    image->width * 4, 0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);
    cursor->id = image->cursorId;
    cursor->cursor = SDL_CreateColorCursor(surface, image->hotX, image->hotY);
    SDL_FreeSurface(surface);
}

static void session_log(IHS_LogLevel level, const char *message) {
    if (level >= IHS_BaseLogLevelWarn) {
        fprintf(stderr, "[IHSSession] %s\n", message);
    } else if (level >= IHS_BaseLogLevelInfo) {
        printf("[IHSSession] %s\n", message);
    }
}
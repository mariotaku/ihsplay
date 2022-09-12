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
    SDL_Cursor *blank_cursor;
    uint64_t cursor_id;
    bool cursor_visible;
} session_fragment_t;

typedef struct cursor_t {
    uint64_t id;
    SDL_Cursor *cursor;
} cursor_t;

static void constructor(lv_fragment_t *self, void *args);

static void destructor(lv_fragment_t *self);

static lv_obj_t *create_obj(lv_fragment_t *self, lv_obj_t *container);

static void session_log(IHS_LogLevel level, const char *tag, const char *message);

const lv_fragment_class_t session_fragment_class = {
        .constructor_cb = constructor,
        .destructor_cb = destructor,
        .create_obj_cb = create_obj,
        .instance_size = sizeof(session_fragment_t)
};

static void session_initialized(IHS_Session *session, void *context);

static void session_connected(IHS_Session *session, void *context);

static void session_disconnected(IHS_Session *session, void *context);

static void session_connected_main(app_t *app, void *context);

static void session_disconnected_main(app_t *app, void *context);

static void session_show_cursor(IHS_Session *session, float x, float y, void *context);

static void session_hide_cursor(IHS_Session *session, void *context);

static bool session_set_cursor(IHS_Session *session, uint64_t cursorId, void *context);

static void session_cursor_image(IHS_Session *session, const IHS_StreamInputCursorImage *image, void *context);

static const cursor_t *session_current_cursor(session_fragment_t *fragment);

static const IHS_StreamSessionCallbacks session_callbacks = {
        .initialized = session_initialized,
        .connected = session_connected,
        .disconnected = session_disconnected,
};
static const IHS_StreamInputCallbacks input_callbacks = {
        .showCursor = session_show_cursor,
        .hideCursor = session_hide_cursor,
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
    fragment->app->active_session = session;
    const static Uint8 blank_pixel[1] = {0};
    fragment->blank_cursor = SDL_CreateCursor(blank_pixel, blank_pixel, 1, 1, 0, 0);
}

static void destructor(lv_fragment_t *self) {
    session_fragment_t *fragment = (session_fragment_t *) self;
    IHS_SessionThreadedJoin(fragment->session);
    IHS_SessionDestroy(fragment->session);
    fragment->app->active_session = NULL;
    array_list_destroy(fragment->cursors);
    SDL_FreeCursor(fragment->blank_cursor);
}

static lv_obj_t *create_obj(lv_fragment_t *self, lv_obj_t *container) {
    return lv_obj_create(container);
}

static void session_initialized(IHS_Session *session, void *context) {
    LV_UNUSED(context);
    IHS_SessionConnect(session);
}

static void session_connected(IHS_Session *session, void *context) {
    LV_UNUSED(session);
    session_fragment_t *fragment = context;
    app_run_on_main(fragment->app, session_connected_main, context);
}

static void session_disconnected(IHS_Session *session, void *context) {
    LV_UNUSED(session);
    session_fragment_t *fragment = context;
    app_run_on_main(fragment->app, session_disconnected_main, context);
}

static void session_connected_main(app_t *app, void *context) {
    LV_UNUSED(context);
    SDL_SetRelativeMouseMode(SDL_TRUE);
}

static void session_disconnected_main(app_t *app, void *context) {
    LV_UNUSED(context);
    SDL_SetRelativeMouseMode(SDL_FALSE);
    SDL_SetCursor(SDL_GetDefaultCursor());
    app_ui_pop_fragment(app->ui);
}

static void session_show_cursor(IHS_Session *session, float x, float y, void *context) {
    session_fragment_t *fragment = context;
//    int w = 0, h = 0;
//    SDL_GetWindowSize(fragment->app->ui->window, &w, &h);
//    SDL_WarpMouseInWindow(fragment->app->ui->window, (int) (x * w), (int) (y * h));
    if (!fragment->cursor_visible) {
        fragment->cursor_visible = true;
        const cursor_t *cursor = session_current_cursor(fragment);
        if (cursor != NULL) {
            SDL_SetCursor(cursor->cursor);
        }
    }
}

static bool session_set_cursor(IHS_Session *session, uint64_t cursorId, void *context) {
    session_fragment_t *fragment = context;
    fragment->cursor_id = cursorId;
    const cursor_t *cursor = session_current_cursor(fragment);
    if (!cursor) {
        return false;
    }
    if (fragment->cursor_visible) {
        SDL_SetCursor(cursor->cursor);
    }
    return true;
}

static void session_hide_cursor(IHS_Session *session, void *context) {
    session_fragment_t *fragment = context;
    if (fragment->cursor_visible) {
        fragment->cursor_visible = false;
        SDL_SetCursor(fragment->blank_cursor);
    }
}

static const cursor_t *session_current_cursor(session_fragment_t *fragment) {
    cursor_t *cursor = NULL;
    for (int i = 0, j = array_list_size(fragment->cursors); i < j; ++i) {
        cursor_t *item = array_list_get(fragment->cursors, i);
        if (item->id == fragment->cursor_id) {
            cursor = item;
            break;
        }
    }
    return cursor;
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

    if (fragment->cursor_visible && fragment->cursor_id == cursor->id) {
        SDL_SetCursor(cursor->cursor);
    }
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
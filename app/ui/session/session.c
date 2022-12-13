#include "session.h"
#include "ihslib.h"
#include "ui/app_ui.h"
#include "app.h"
#include "ui/common/progress_dialog.h"
#include "util/array_list.h"
#include "backend/stream_manager.h"
#include "lvgl/fonts/material-icons/symbols.h"
#include "streaming_overlay.h"

typedef struct session_fragment_t {
    lv_fragment_t base;
    app_t *app;
    IHS_HostInfo host;

    lv_obj_t *progress;

    array_list_t *cursors;
    SDL_Cursor *blank_cursor;
    uint64_t cursor_id;
    bool cursor_visible;

    lv_fragment_t *overlay;
} session_fragment_t;

typedef struct cursor_t {
    uint64_t id;
    SDL_Cursor *cursor;
} cursor_t;

static void constructor(lv_fragment_t *self, void *args);

static void destructor(lv_fragment_t *self);

static lv_obj_t *create_obj(lv_fragment_t *self, lv_obj_t *container);

static void obj_created(lv_fragment_t *self, lv_obj_t *obj);

static void obj_will_delete(lv_fragment_t *self, lv_obj_t *obj);

static bool event_cb(lv_fragment_t *self, int code, void *userdata);

const lv_fragment_class_t session_fragment_class = {
        .constructor_cb = constructor,
        .destructor_cb = destructor,
        .create_obj_cb = create_obj,
        .obj_created_cb = obj_created,
        .obj_will_delete_cb = obj_will_delete,
        .event_cb = event_cb,
        .instance_size = sizeof(session_fragment_t)
};

static void session_connected_main(const IHS_SessionInfo *info, void *context);

static void session_disconnected_main(const IHS_SessionInfo *info, bool requested, void *context);

const static stream_manager_listener_t stream_manager_listener = {
        .connected = session_connected_main,
        .disconnected = session_disconnected_main,
};

static void session_show_cursor(IHS_Session *session, float x, float y, void *context);

static void session_hide_cursor(IHS_Session *session, void *context);

static bool session_set_cursor(IHS_Session *session, uint64_t cursorId, void *context);

static void session_cursor_image(IHS_Session *session, const IHS_StreamInputCursorImage *image, void *context);

static const cursor_t *session_current_cursor(session_fragment_t *fragment);

static void disconnected_dialog_cb(lv_event_t *e);

static void screen_clicked_cb(lv_event_t *e);

static void set_overlay_visible(session_fragment_t *fragment, bool visible);

static const IHS_StreamInputCallbacks input_callbacks = {
        .showCursor = session_show_cursor,
        .hideCursor = session_hide_cursor,
        .setCursor = session_set_cursor,
        .cursorImage = session_cursor_image,
};

static void constructor(lv_fragment_t *self, void *args) {
    session_fragment_t *fragment = (session_fragment_t *) self;
    const app_ui_fragment_args_t *fargs = args;
    fragment->app = fargs->app;
    fragment->host = *(const IHS_HostInfo *) fargs->data;
    fragment->cursors = array_list_create(sizeof(cursor_t), 16);
    const static Uint8 blank_pixel[1] = {0};
    fragment->blank_cursor = SDL_CreateCursor(blank_pixel, blank_pixel, 1, 1, 0, 0);
}

static void destructor(lv_fragment_t *self) {
    session_fragment_t *fragment = (session_fragment_t *) self;
    array_list_destroy(fragment->cursors);
    SDL_FreeCursor(fragment->blank_cursor);
}

static lv_obj_t *create_obj(lv_fragment_t *self, lv_obj_t *container) {
    session_fragment_t *fragment = (session_fragment_t *) self;
    lv_obj_t *obj = lv_obj_create(container);
    lv_obj_remove_style_all(obj);
    lv_obj_set_size(obj, LV_PCT(100), LV_PCT(100));
    lv_obj_add_event_cb(obj, screen_clicked_cb, LV_EVENT_CLICKED, fragment);
    return obj;
}

static void obj_created(lv_fragment_t *self, lv_obj_t *obj) {
    session_fragment_t *fragment = (session_fragment_t *) self;

    fragment->progress = progress_dialog_create("Requesting stream");
    stream_manager_t *stream_manager = fragment->app->stream_manager;
    stream_manager_register_listener(stream_manager, &stream_manager_listener, fragment);
    stream_manager_start(stream_manager, &fragment->host);

    app_ui_set_ignore_keys(fragment->app->ui, true);
}

static void obj_will_delete(lv_fragment_t *self, lv_obj_t *obj) {
    session_fragment_t *fragment = (session_fragment_t *) self;
    app_ui_set_ignore_keys(fragment->app->ui, false);

    if (fragment->progress != NULL) {
        lv_msgbox_close(fragment->progress);
        fragment->progress = NULL;
    }

    stream_manager_unregister_listener(fragment->app->stream_manager, &stream_manager_listener);
}

static bool event_cb(lv_fragment_t *self, int code, void *userdata) {
    session_fragment_t *fragment = (session_fragment_t *) self;
    switch (code) {
        case APP_UI_REQUEST_OVERLAY: {
            set_overlay_visible(fragment, true);
            return true;
        }
        case APP_UI_CLOSE_OVERLAY: {
            set_overlay_visible(fragment, false);
            return true;
        }
    }
    return false;
}

static void session_connected_main(const IHS_SessionInfo *info, void *context) {
    LV_UNUSED(info);
    session_fragment_t *fragment = (session_fragment_t *) context;
//    SDL_SetRelativeMouseMode(SDL_TRUE);

    if (fragment->progress != NULL) {
        lv_msgbox_close(fragment->progress);
        fragment->progress = NULL;
    }
}

static void session_disconnected_main(const IHS_SessionInfo *info, bool requested, void *context) {
    LV_UNUSED(info);
    session_fragment_t *fragment = (session_fragment_t *) context;
//    SDL_SetRelativeMouseMode(SDL_FALSE);
//    SDL_SetCursor(SDL_GetDefaultCursor());
    if (!requested) {
        static const char *btn_txts[] = {"OK", ""};
        lv_obj_t *mbox = lv_msgbox_create(NULL, NULL, "Disconnected.", btn_txts, false);
        lv_obj_add_event_cb(mbox, disconnected_dialog_cb, LV_EVENT_VALUE_CHANGED, NULL);
        lv_obj_center(mbox);
    }
    lv_fragment_manager_pop(fragment->app->ui->fm);
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
//            SDL_SetCursor(cursor->cursor);
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
//        SDL_SetCursor(cursor->cursor);
    }
    return true;
}

static void session_hide_cursor(IHS_Session *session, void *context) {
    session_fragment_t *fragment = context;
    if (fragment->cursor_visible) {
        fragment->cursor_visible = false;
//        SDL_SetCursor(fragment->blank_cursor);
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
//        SDL_SetCursor(cursor->cursor);
    }
}

static void disconnected_dialog_cb(lv_event_t *e) {
    lv_obj_t *msgbox = lv_event_get_current_target(e);
    lv_msgbox_close_async(msgbox);
}

static void screen_clicked_cb(lv_event_t *e) {
    session_fragment_t *fragment = lv_event_get_user_data(e);
    stream_manager_t *manager = fragment->app->stream_manager;
    if (!stream_manager_is_overlay_opened(manager)) {
        return;
    }
    stream_manager_set_overlay_opened(manager, false);
}

static void set_overlay_visible(session_fragment_t *fragment, bool visible) {
    if (visible == (fragment->overlay != NULL)) {
        return;
    }
    if (visible) {
        lv_fragment_t *overlay_fragment = lv_fragment_create(&streaming_overlay_class, fragment->app);
        lv_fragment_manager_replace(fragment->base.child_manager, overlay_fragment, &fragment->base.obj);
        fragment->overlay = overlay_fragment;
    } else {
        lv_fragment_manager_remove(fragment->base.child_manager, fragment->overlay);
        fragment->overlay = NULL;
    }
}
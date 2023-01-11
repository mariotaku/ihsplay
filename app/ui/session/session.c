#include <assert.h>
#include "session.h"
#include "ihslib.h"
#include "ui/app_ui.h"
#include "app.h"
#include "ui/common/progress_dialog.h"
#include "util/array_list.h"
#include "backend/stream_manager.h"
#include "streaming_overlay.h"
#include "backend/host_manager.h"
#include "connection_progress.h"
#include "backend/input_manager.h"
#include "logging/app_logging.h"
#include "config.h"

typedef struct session_fragment_t {
    lv_fragment_t base;
    app_t *app;
    session_fragment_args_t args;

    array_list_t *cursors;
    SDL_Cursor *blank_cursor;
    uint64_t cursor_id;
    bool cursor_visible;

    lv_fragment_t *overlay;

    lv_obj_t *overlay_hint;
    lv_obj_t *overlay_progress;

    struct {
        lv_style_t overlay;
    } styles;
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

static void session_overlay_progress(int percentage, void *context);

static void session_overlay_progress_finished(bool requested, void *context);

const static stream_manager_listener_t stream_manager_listener = {
        .connected = session_connected_main,
        .disconnected = session_disconnected_main,
        .overlay_progress = session_overlay_progress,
        .overlay_progress_finished = session_overlay_progress_finished,
};


static void session_show_cursor(IHS_Session *session, float x, float y, void *context);

static void session_hide_cursor(IHS_Session *session, void *context);

static bool session_set_cursor(IHS_Session *session, uint64_t cursorId, void *context);

static void session_cursor_image(IHS_Session *session, const IHS_StreamInputCursorImage *image, void *context);

static const cursor_t *session_current_cursor(session_fragment_t *fragment);

static void disconnected_dialog_cb(lv_event_t *e);

static void screen_clicked_cb(lv_event_t *e);

static void set_overlay_visible(session_fragment_t *fragment, bool visible);

static void constructor(lv_fragment_t *self, void *args) {
    session_fragment_t *fragment = (session_fragment_t *) self;
    const app_ui_fragment_args_t *fargs = args;
    fragment->app = fargs->app;
#if IHSPLAY_IS_DEBUG
    if (fargs->data == NULL) {
        memset(&fragment->args, 0, sizeof(fragment->args));
    } else {
        fragment->args = *(session_fragment_args_t *) fargs->data;
    }
#else
    assert (fargs->data != NULL);
    fragment->args = *(session_fragment_args_t *) fargs->data;
#endif
    fragment->cursors = array_list_create(sizeof(cursor_t), 16);
    const static Uint8 blank_pixel[1] = {0};
    fragment->blank_cursor = SDL_CreateCursor(blank_pixel, blank_pixel, 1, 1, 0, 0);

    lv_coord_t overlay_height = LV_DPX(100);

    lv_style_init(&fragment->styles.overlay);
    lv_style_set_border_side(&fragment->styles.overlay, LV_BORDER_SIDE_TOP);
    lv_style_set_border_width(&fragment->styles.overlay, LV_DPX(2));
    lv_style_set_border_color(&fragment->styles.overlay, lv_palette_main(LV_PALETTE_BLUE));
    lv_style_set_bg_color(&fragment->styles.overlay, lv_palette_main(LV_PALETTE_BLUE_GREY));
    lv_style_set_bg_opa(&fragment->styles.overlay, LV_OPA_80);
    lv_style_set_pad_ver(&fragment->styles.overlay, LV_DPX(5));
    lv_style_set_pad_hor(&fragment->styles.overlay, LV_DPX(30));
    lv_style_set_width(&fragment->styles.overlay, LV_PCT(100));
    lv_style_set_height(&fragment->styles.overlay, overlay_height);
    lv_style_set_align(&fragment->styles.overlay, LV_ALIGN_BOTTOM_MID);

    stream_manager_set_overlay_height(fragment->app->stream_manager, overlay_height);
}

static void destructor(lv_fragment_t *self) {
    session_fragment_t *fragment = (session_fragment_t *) self;
    lv_style_reset(&fragment->styles.overlay);
    array_list_destroy(fragment->cursors);
    SDL_FreeCursor(fragment->blank_cursor);
}

static lv_obj_t *create_obj(lv_fragment_t *self, lv_obj_t *container) {
    session_fragment_t *fragment = (session_fragment_t *) self;
    lv_obj_t *obj = lv_obj_create(container);
    lv_obj_remove_style_all(obj);
    lv_obj_set_size(obj, LV_PCT(100), LV_PCT(100));
    lv_obj_add_event_cb(obj, screen_clicked_cb, LV_EVENT_CLICKED, fragment);

    lv_obj_t *overlay_hint = lv_obj_create(obj);
    lv_obj_set_size(overlay_hint, LV_SIZE_CONTENT, LV_DPX(60));
    lv_obj_set_style_bg_opa(overlay_hint, LV_OPA_80, 0);
    lv_obj_set_style_bg_color(overlay_hint, lv_palette_main(LV_PALETTE_BLUE_GREY), 0);
    lv_obj_set_layout(overlay_hint, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(overlay_hint, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(overlay_hint, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_START);
    lv_obj_set_style_pad_ver(overlay_hint, LV_DPX(15), 0);
    lv_obj_set_style_pad_hor(overlay_hint, LV_DPX(20), 0);
    lv_obj_set_style_pad_gap(overlay_hint, LV_DPX(15), 0);

    lv_obj_t *overlay_label = lv_label_create(overlay_hint);
    lv_label_set_text_static(overlay_label, "Long press to open the menu");

    lv_obj_t *overlay_progress = lv_arc_create(overlay_hint);
    lv_obj_set_size(overlay_progress, LV_DPX(30), LV_DPX(30));
    lv_arc_set_bg_angles(overlay_progress, 0, 360);
    lv_arc_set_angles(overlay_progress, 0, 360);
    lv_arc_set_rotation(overlay_progress, 270);
    lv_arc_set_range(overlay_progress, 0, 99);
    lv_arc_set_value(overlay_progress, 30);
    lv_obj_set_style_arc_width(overlay_progress, LV_DPX(5), 0);
    lv_obj_set_style_arc_width(overlay_progress, LV_DPX(5), LV_PART_INDICATOR);

    fragment->overlay_hint = overlay_hint;
    fragment->overlay_progress = overlay_progress;

    lv_obj_add_flag(overlay_hint, LV_OBJ_FLAG_HIDDEN);
    lv_obj_align(overlay_hint, LV_ALIGN_LEFT_MID, 0, LV_PCT(10));

    return obj;
}

static void obj_created(lv_fragment_t *self, lv_obj_t *obj) {
    LV_UNUSED(obj);
    session_fragment_t *fragment = (session_fragment_t *) self;

    fragment->overlay = lv_fragment_create(&connection_progress_class, fragment->app);
    lv_fragment_manager_replace(fragment->base.child_manager, fragment->overlay, &fragment->base.obj);

    stream_manager_t *stream_manager = fragment->app->stream_manager;
    stream_manager_register_listener(stream_manager, &stream_manager_listener, fragment);
    if (fragment->args.session.sessionKeyLen > 0) {
        stream_manager_start_session(stream_manager, &fragment->args.session);
    }

    app_ui_set_ignore_keys(fragment->app->ui, true);

    lv_obj_set_style_bg_opa(lv_scr_act(), LV_OPA_TRANSP, 0);
}

static void obj_will_delete(lv_fragment_t *self, lv_obj_t *obj) {
    LV_UNUSED(obj);
    session_fragment_t *fragment = (session_fragment_t *) self;
    app_ui_set_ignore_keys(fragment->app->ui, false);

    stream_manager_unregister_listener(fragment->app->stream_manager, &stream_manager_listener);

    lv_obj_set_style_bg_opa(lv_scr_act(), LV_OPA_COVER, 0);
}

static bool event_cb(lv_fragment_t *self, int code, void *userdata) {
    LV_UNUSED(userdata);
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
        case APP_UI_NAV_BACK: {
            stream_manager_t *stream_manager = fragment->app->stream_manager;
            if (stream_manager_is_overlay_opened(stream_manager)) {
                stream_manager_set_overlay_opened(stream_manager, false);
                return true;
            }
            return false;
        }
        default:
            break;
    }
    return false;
}

static void session_connected_main(const IHS_SessionInfo *info, void *context) {
    LV_UNUSED(info);
    session_fragment_t *fragment = (session_fragment_t *) context;

    if (fragment->overlay != NULL) {
        lv_fragment_manager_remove(fragment->base.child_manager, fragment->overlay);
        fragment->overlay = NULL;
    }
}

static void session_disconnected_main(const IHS_SessionInfo *info, bool requested, void *context) {
    LV_UNUSED(info);
    session_fragment_t *fragment = (session_fragment_t *) context;
//    SDL_SetCursor(SDL_GetDefaultCursor());
    if (!requested) {
        static const char *btn_txts[] = {"OK", ""};
        lv_obj_t *mbox = lv_msgbox_create(NULL, NULL, "Disconnected.", btn_txts, false);
        lv_obj_add_event_cb(mbox, disconnected_dialog_cb, LV_EVENT_VALUE_CHANGED, NULL);
        lv_obj_center(mbox);
    }
    app_ui_pop_top_fragment(fragment->app->ui);
}

static void session_overlay_progress(int percentage, void *context) {
    session_fragment_t *fragment = (session_fragment_t *) context;
    if (lv_obj_has_flag(fragment->overlay_hint, LV_OBJ_FLAG_HIDDEN)) {
        lv_obj_clear_flag(fragment->overlay_hint, LV_OBJ_FLAG_HIDDEN);
    }
    lv_arc_set_value(fragment->overlay_progress, (int16_t) percentage);
}

static void session_overlay_progress_finished(bool requested, void *context) {
    session_fragment_t *fragment = (session_fragment_t *) context;
    lv_obj_add_flag(fragment->overlay_hint, LV_OBJ_FLAG_HIDDEN);
}

static void session_show_cursor(IHS_Session *session, float x, float y, void *context) {
    session_fragment_t *fragment = context;
    app_log_info("Session", "show_cursor: x=%f, y=%f", x, y);

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
    if (visible == (fragment->overlay != NULL && fragment->overlay->cls == &streaming_overlay_class)) {
        return;
    }
    app_ui_set_ignore_keys(fragment->app->ui, !visible);
    if (visible) {
        lv_fragment_t *overlay_fragment = lv_fragment_create(&streaming_overlay_class, fragment->app);
        lv_fragment_manager_replace(fragment->base.child_manager, overlay_fragment, &fragment->base.obj);
        fragment->overlay = overlay_fragment;
    } else {
        lv_fragment_manager_remove(fragment->base.child_manager, fragment->overlay);
        fragment->overlay = NULL;
    }
}

lv_style_t *session_fragment_get_overlay_style(lv_fragment_t *fragment) {
    return &((session_fragment_t *) fragment)->styles.overlay;
}

const char *session_fragment_get_host_name(lv_fragment_t *fragment) {
    return ((session_fragment_t *) fragment)->args.host.hostname;
}
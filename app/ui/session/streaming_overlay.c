#include "streaming_overlay.h"
#include "lvgl/fonts/bootstrap-icons/symbols.h"

#include "app.h"
#include "ui/app_ui.h"
#include "backend/stream_manager.h"
#include "session.h"

typedef struct streaming_overlay_fragment_t {
    lv_fragment_t base;
    app_t *app;
    struct {
    } styles;
} streaming_overlay_fragment_t;

static void constructor_cb(lv_fragment_t *self, void *args);

static void destructor_cb(lv_fragment_t *self);

static lv_obj_t *create_obj_cb(lv_fragment_t *self, lv_obj_t *container);

static void obj_created_cb(lv_fragment_t *self, lv_obj_t *obj);

static void quit_clicked_cb(lv_event_t *e);

const lv_fragment_class_t streaming_overlay_class = {
        .constructor_cb = constructor_cb,
        .destructor_cb = destructor_cb,
        .create_obj_cb = create_obj_cb,
        .obj_created_cb = obj_created_cb,
        .instance_size = sizeof(streaming_overlay_fragment_t),
};

static void constructor_cb(lv_fragment_t *self, void *args) {
    streaming_overlay_fragment_t *fragment = (streaming_overlay_fragment_t *) self;
    fragment->app = args;

}

static void destructor_cb(lv_fragment_t *self) {
}

static lv_obj_t *create_obj_cb(lv_fragment_t *self, lv_obj_t *container) {
    streaming_overlay_fragment_t *fragment = (streaming_overlay_fragment_t *) self;
    lv_fragment_t *session_fragment = lv_fragment_get_parent(self);
    lv_obj_t *content = lv_obj_create(container);
    lv_obj_remove_style_all(content);
    lv_obj_add_style(content, session_fragment_get_overlay_style(session_fragment), 0);
    lv_obj_set_style_pad_hor(content, LV_DPX(30), 0);
    lv_obj_add_flag(content, LV_OBJ_FLAG_CLICKABLE);

    lv_obj_t *quit = lv_btn_create(content);
    lv_obj_set_style_radius(quit, LV_DPX(4), 0);
    lv_obj_add_event_cb(quit, quit_clicked_cb, LV_EVENT_CLICKED, self);
    lv_obj_t *quit_label = lv_label_create(quit);
    lv_obj_add_style(quit_label, &fragment->app->ui->styles.action_btn_label, 0);
    lv_label_set_text(quit_label, BS_SYMBOL_POWER);

    lv_obj_align(quit, LV_ALIGN_LEFT_MID, 0, 0);

    return content;
}

static void obj_created_cb(lv_fragment_t *self, lv_obj_t *obj) {

}

static void quit_clicked_cb(lv_event_t *e) {
    streaming_overlay_fragment_t *fragment = lv_event_get_user_data(e);
    stream_manager_stop_active(fragment->app->stream_manager);
}
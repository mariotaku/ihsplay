#include <assert.h>
#include "app.h"
#include "app_ui.h"
#include "config.h"

#include "launcher.h"

#include "hosts/hosts_fragment.h"
#include "settings/settings.h"

#include "lvgl/fonts/bootstrap-icons/symbols.h"

#include "ui/settings/basic.h"
#include "ui/common/group_utils.h"
#include "ui/support/support.h"

#include "backend/host_manager.h"

typedef struct launcher_fragment {
    lv_fragment_t base;
    app_t *app;
    lv_coord_t row_dsc[5], col_dsc[4];
    struct {
        lv_style_t root;
        lv_style_t launch_option_label;
    } styles;
    lv_obj_t *nav_content;
    lv_obj_t *stream_interface_icon;
    int num_launch_options;
    IHS_StreamInterface stream_interface;
#if IHSPLAY_IS_DEBUG
    lv_obj_t *debug_info;
#endif
} launcher_fragment;

static void constructor(lv_fragment_t *self, void *arg);

static void destructor(lv_fragment_t *self);

static lv_obj_t *create_obj(lv_fragment_t *self, lv_obj_t *container);

static void obj_created(lv_fragment_t *self, lv_obj_t *obj);

static void obj_will_delete(lv_fragment_t *self, lv_obj_t *obj);

static bool event_cb(lv_fragment_t *self, int type, void *data);

static lv_obj_t *launch_option_create_label_action(launcher_fragment *fragment, const char *icon, const char *label);

static lv_obj_t *launch_option_create_dropdown_action(launcher_fragment *fragment, const char *icon,
                                                      lv_obj_t **icon_obj_out);

static void focus_content(lv_event_t *e);

static void launcher_hosts(lv_event_t *e);

static void open_settings(lv_event_t *e);

static void open_support(lv_event_t *e);

static void launcher_quit(lv_event_t *e);

static void stream_interface_change_cb(lv_event_t *e);

static void focus_first_in_parent(launcher_fragment *fragment, lv_obj_t *parent);

const lv_fragment_class_t launcher_fragment_class = {
        .constructor_cb = constructor,
        .destructor_cb = destructor,
        .create_obj_cb = create_obj,
        .obj_created_cb = obj_created,
        .obj_will_delete_cb = obj_will_delete,
        .event_cb = event_cb,
        .instance_size = sizeof(launcher_fragment),
};

static const char *stream_interface_icons[] = {
        BS_SYMBOL_PLAY_BTN,
        BS_SYMBOL_COLLECTION,
        BS_SYMBOL_STEAM,
        BS_SYMBOL_WINDOW_DESKTOP
};

void launcher_fragment_focus_content(lv_fragment_t *self) {
    if (self->cls != &launcher_fragment_class) {
        return;
    }
    launcher_fragment *fragment = (launcher_fragment *) self;
    focus_first_in_parent(fragment, fragment->nav_content);
}

static void constructor(lv_fragment_t *self, void *arg) {
    app_ui_fragment_args_t *fargs = arg;
    launcher_fragment *fragment = (launcher_fragment *) self;
    fragment->app = fargs->app;
    fragment->col_dsc[0] = LV_DPX(200);
    fragment->col_dsc[1] = LV_DPX(40);
    fragment->col_dsc[2] = LV_DPX(300);
    fragment->col_dsc[3] = LV_GRID_TEMPLATE_LAST;
    fragment->row_dsc[0] = LV_DPX(40);
    fragment->row_dsc[1] = LV_DPX(40);
    fragment->row_dsc[2] = LV_DPX(40);
    fragment->row_dsc[3] = LV_GRID_FR(1);
    fragment->row_dsc[4] = LV_GRID_TEMPLATE_LAST;
    fragment->stream_interface = IHS_StreamInterfaceDefault;

    lv_style_init(&fragment->styles.root);
    lv_style_set_pad_gap(&fragment->styles.root, LV_DPX(10));
    lv_style_set_pad_hor(&fragment->styles.root, 0);
    lv_style_set_pad_top(&fragment->styles.root, LV_DPX(40));
    lv_style_set_pad_bottom(&fragment->styles.root, 0);

    lv_style_init(&fragment->styles.launch_option_label);
    lv_style_set_text_font(&fragment->styles.launch_option_label, fragment->app->ui->font.heading3);
}

static void destructor(lv_fragment_t *self) {
    launcher_fragment *fragment = (launcher_fragment *) self;
    lv_style_reset(&fragment->styles.launch_option_label);
    lv_style_reset(&fragment->styles.root);
}

static lv_obj_t *create_obj(lv_fragment_t *self, lv_obj_t *container) {
    launcher_fragment *fragment = (launcher_fragment *) self;
    fragment->num_launch_options = 0;
    lv_obj_t *win = lv_win_create(container, LV_SIZE_CONTENT);

    lv_win_add_title(win, "IHSplay");

//    lv_obj_add_event_cb(actions, focus_content, LV_EVENT_KEY, fragment);

#if IHSPLAY_WIP_FEATURES
    lv_obj_t *btn_settings = lv_win_add_btn(win, BS_SYMBOL_GEAR_FILL, LV_DPX(40));
    lv_obj_add_event_cb(btn_settings, open_settings, LV_EVENT_CLICKED, fragment);
#endif

    lv_obj_t *btn_support = lv_win_add_btn(win, BS_SYMBOL_QUESTION_CIRCLE_FILL, LV_DPX(40));
    lv_obj_add_event_cb(btn_support, open_support, LV_EVENT_CLICKED, fragment);

    lv_obj_t *btn_quit = lv_win_add_btn(win, BS_SYMBOL_X_LG, LV_DPX(40));
    lv_obj_add_event_cb(btn_quit, launcher_quit, LV_EVENT_CLICKED, fragment->app);

    lv_obj_t *nav_content = lv_win_get_content(win);
    fragment->nav_content = nav_content;

    lv_obj_set_layout(nav_content, LV_LAYOUT_GRID);
    lv_obj_set_grid_dsc_array(nav_content, fragment->col_dsc, fragment->row_dsc);
    lv_obj_set_style_pad_gap(nav_content, LV_DPX(20), 0);

    lv_obj_t *btn_play = lv_btn_create(nav_content);
    lv_obj_set_size(btn_play, LV_SIZE_CONTENT, LV_DPX(150));
    lv_obj_set_flex_flow(btn_play, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(btn_play, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_t *img_play = lv_label_create(btn_play);
    lv_obj_set_style_text_font(img_play, fragment->app->ui->iconfont.heading1, 0);
    lv_label_set_text_static(img_play, BS_SYMBOL_PLAY_CIRCLE_FILL);
    lv_obj_t *label_play = lv_label_create(btn_play);
    lv_label_set_text(label_play, "Start Streaming");
    lv_obj_set_grid_cell(btn_play, LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_START, 0, 4);

    launch_option_create_label_action(fragment, BS_SYMBOL_DISPLAY, "HOSTNAME");
    launch_option_create_label_action(fragment, BS_SYMBOL_CONTROLLER, "No Gamepad connected");

    lv_obj_t *stream_interface = launch_option_create_dropdown_action(fragment, LV_SYMBOL_DUMMY,
                                                                      &fragment->stream_interface_icon);
    lv_label_set_text(fragment->stream_interface_icon, stream_interface_icons[fragment->stream_interface]);
    lv_obj_add_event_cb(stream_interface, stream_interface_change_cb, LV_EVENT_VALUE_CHANGED, fragment);
    lv_dropdown_set_options_static(stream_interface, "Default\nRecent Games\nBig Picture\nDesktop");
    lv_dropdown_set_selected(stream_interface, fragment->stream_interface);

#if IHSPLAY_IS_DEBUG
    lv_obj_t *debug_info = lv_label_create(container);
    lv_obj_set_style_pad_all(debug_info, LV_DPX(10), 0);
    lv_obj_set_style_bg_color(debug_info, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(debug_info, LV_OPA_30, 0);
    lv_obj_set_style_text_opa(debug_info, LV_OPA_50, 0);
    lv_label_set_text_fmt(debug_info, "audio_driver: %s\nvideo_driver: %s", fragment->app->settings->audio_driver,
                          fragment->app->settings->video_driver);
    lv_obj_align(debug_info, LV_ALIGN_BOTTOM_RIGHT, 0, 0);

    fragment->debug_info = debug_info;
#endif

    return win;
}

static void obj_created(lv_fragment_t *self, lv_obj_t *obj) {
    launcher_fragment *fragment = (launcher_fragment *) self;
//    lv_fragment_t *f = lv_fragment_manager_find_by_container(self->child_manager, fragment->nav_content);
//    assert(f != NULL);
//    hosts_fragment_focus_hosts(f);
}

static void obj_will_delete(lv_fragment_t *self, lv_obj_t *obj) {
#if IHSPLAY_IS_DEBUG
    launcher_fragment *fragment = (launcher_fragment *) self;
    lv_obj_del(fragment->debug_info);
#endif
}

static bool event_cb(lv_fragment_t *self, int type, void *data) {
    (void) data;
    launcher_fragment *fragment = (launcher_fragment *) self;
    switch (type) {
        case APP_UI_NAV_QUIT:
            app_quit(fragment->app);
            return true;
        case APP_UI_NAV_BACK:
            return true;
        default:
            return false;
    }
}

static lv_obj_t *launch_option_create_label_action(launcher_fragment *fragment, const char *icon, const char *label) {
    int row_pos = fragment->num_launch_options++;
    lv_obj_t *action = lv_label_create(fragment->nav_content);
    lv_obj_set_grid_cell(action, LV_GRID_ALIGN_STRETCH, 2, 1, LV_GRID_ALIGN_CENTER, row_pos, 1);
    lv_obj_set_size(action, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_label_set_text(action, label);

    lv_obj_t *icon_obj = lv_label_create(fragment->nav_content);
    lv_obj_add_style(icon_obj, &fragment->styles.launch_option_label, 0);
    lv_label_set_text(icon_obj, icon);
    lv_obj_set_grid_cell(icon_obj, LV_GRID_ALIGN_CENTER, 1, 1, LV_GRID_ALIGN_CENTER, row_pos, 1);

    return action;
}

static lv_obj_t *launch_option_create_dropdown_action(launcher_fragment *fragment, const char *icon,
                                                      lv_obj_t **icon_obj_out) {
    int row_pos = fragment->num_launch_options++;
    lv_obj_t *action = lv_dropdown_create(fragment->nav_content);
    lv_obj_set_grid_cell(action, LV_GRID_ALIGN_STRETCH, 1, 2, LV_GRID_ALIGN_CENTER, row_pos, 1);
    lv_obj_set_size(action, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_style_pad_left(action, fragment->row_dsc[1] + lv_obj_get_style_pad_column(fragment->nav_content, 0), 0);

    lv_obj_t *icon_obj = lv_label_create(fragment->nav_content);
    lv_obj_add_style(icon_obj, &fragment->styles.launch_option_label, 0);
    lv_label_set_text(icon_obj, icon);
    lv_obj_set_grid_cell(icon_obj, LV_GRID_ALIGN_CENTER, 1, 1, LV_GRID_ALIGN_CENTER, row_pos, 1);
    if (icon_obj_out != NULL) {
        *icon_obj_out = icon_obj;
    }
    return action;
}


static void focus_content(lv_event_t *e) {
    lv_obj_t *current_target = lv_event_get_current_target(e);
    lv_obj_t *target = lv_event_get_target(e);
    if (lv_obj_get_parent(target) != current_target) {
        return;
    }
    switch (lv_event_get_key(e)) {
        case LV_KEY_DOWN: {
            lv_fragment_t *fragment = lv_event_get_user_data(e);
            launcher_fragment_focus_content(fragment);
            break;
        }
        case LV_KEY_LEFT:
        case LV_KEY_RIGHT: {
            int focused_action = -1;
            uint32_t action_count = lv_obj_get_child_cnt(current_target);
            for (int i = 0, j = (int) action_count; i < j; i++) {
                if (target == lv_obj_get_child(current_target, i)) {
                    focused_action = i;
                    break;
                }
            }
            focused_action += lv_event_get_key(e) == LV_KEY_LEFT ? -1 : 1;
            if (focused_action < 0) {
                focused_action = (int) (action_count - 1);
            } else if (focused_action >= action_count) {
                focused_action = 0;
            }
            lv_group_focus_obj(lv_obj_get_child(current_target, focused_action));
            break;
        }
    }
}

static void open_settings(lv_event_t *e) {
    launcher_fragment *fragment = lv_event_get_user_data(e);
    app_ui_push_fragment(fragment->app->ui, &settings_basic_fragment_class, fragment->app);
}

static void open_support(lv_event_t *e) {
    launcher_fragment *fragment = lv_event_get_user_data(e);
    app_ui_push_fragment(fragment->app->ui, &support_fragment_class, fragment->app);
}

static void launcher_quit(lv_event_t *e) {
    app_quit(lv_event_get_user_data(e));
}

static void stream_interface_change_cb(lv_event_t *e) {
    launcher_fragment *fragment = lv_event_get_user_data(e);
    lv_obj_t *dropdown = lv_event_get_current_target(e);
    fragment->stream_interface = lv_dropdown_get_selected(dropdown);
    assert(fragment->stream_interface <= IHS_StreamInterfaceDesktop);
    lv_label_set_text(fragment->stream_interface_icon, stream_interface_icons[fragment->stream_interface]);
}

static void focus_first_in_parent(launcher_fragment *fragment, lv_obj_t *parent) {
    lv_group_t *group = app_ui_get_input_group(fragment->app->ui);
    if (group == NULL) {
        return;
    }
    lv_obj_t *to_focus = ui_group_first_in_parent(group, parent);
    if (to_focus == NULL) {
        return;
    }
    lv_group_focus_obj(to_focus);
}

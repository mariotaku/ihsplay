#include "hosts_fragment.h"

#include "app.h"
#include "backend/host_manager.h"
#include "lv_gridview.h"
#include "ui/app_ui.h"
#include "ui/session/session.h"
#include "util/array_list.h"
#include "lvgl/fonts/bootstrap-icons/symbols.h"
#include "util/random.h"
#include "lvgl/ext/msgbox_ext.h"
#include "ui/common/error_messages.h"
#include "logging/app_logging.h"
#include "lvgl/theme.h"
#include "ui/launcher.h"

typedef struct hosts_fragment {
    lv_fragment_t base;
    app_t *app;
    lv_fragment_t *launcher_fragment;
    lv_obj_t *grid_view;
    lv_obj_t *msgbox;
} hosts_fragment;

typedef struct host_obj_holder {
    lv_obj_t *icon;
    lv_obj_t *os_icon;
    lv_obj_t *name;
} host_obj_holder;

static void constructor(lv_fragment_t *self, void *arg);

static void destructor(lv_fragment_t *self);

static lv_obj_t *create_obj(lv_fragment_t *self, lv_obj_t *container);

static void obj_created(lv_fragment_t *self, lv_obj_t *obj);

static void obj_will_delete(lv_fragment_t *self, lv_obj_t *obj);

static void obj_deleted(lv_fragment_t *self, lv_obj_t *obj);

static bool event_cb(lv_fragment_t *self, int code, void *data);

static void hosts_changed(array_list_t *list, host_manager_hosts_change change_type, int change_index, void *context);

static int host_item_count(lv_obj_t *grid, void *data);

static lv_obj_t *host_item_create(lv_obj_t *grid);

static void host_item_delete(lv_event_t *e);

static void host_item_clicked(lv_event_t *e);

static void size_changed_cb(lv_event_t *e);

static void grid_focused(lv_event_t *e);

static void grid_unfocused(lv_event_t *e);

static void grid_key_cb(lv_event_t *e);

static void host_item_bind(lv_obj_t *grid, lv_obj_t *item_view, void *data, int position);

static void grid_size_populate(hosts_fragment *fragment);

static lv_obj_t *open_msgbox(hosts_fragment *fragment, const char *title, const char *message, const char *btns[]);

static void close_msgbox(hosts_fragment *fragment);

static void msgbox_confirm_cb(lv_event_t *e);

static void msgbox_del_cb(lv_event_t *e);

static void authorization_cancel_cb(lv_event_t *e);

const lv_fragment_class_t hosts_fragment_class = {
        .constructor_cb = constructor,
        .destructor_cb = destructor,
        .create_obj_cb = create_obj,
        .obj_created_cb = obj_created,
        .obj_will_delete_cb = obj_will_delete,
        .obj_deleted_cb = obj_deleted,
        .event_cb = event_cb,
        .instance_size = sizeof(hosts_fragment),
};

static const host_manager_listener_t host_manager_listener = {
        .hosts_changed = hosts_changed,
};

static const lv_gridview_adapter_t hosts_adapter = {
        .item_count = host_item_count,
        .create_view = host_item_create,
        .bind_view = host_item_bind,
};

void hosts_fragment_focus_hosts(lv_fragment_t *self) {
    if (self->cls != &hosts_fragment_class) {
        return;
    }
    hosts_fragment *fragment = (hosts_fragment *) self;
    lv_group_focus_obj(fragment->grid_view);
    lv_gridview_focus_when_available(fragment->grid_view, 0);
}

static void constructor(lv_fragment_t *self, void *arg) {
    hosts_fragment *fragment = (hosts_fragment *) self;
    app_ui_fragment_args_t *args = arg;
    fragment->app = args->app;
    fragment->launcher_fragment = args->data;
}

static void destructor(lv_fragment_t *self) {
    LV_UNUSED(self);
}

static lv_obj_t *create_obj(lv_fragment_t *self, lv_obj_t *container) {
    hosts_fragment *fragment = (hosts_fragment *) self;
    lv_obj_t *win = app_lv_win_create(container);
    lv_win_add_title(win, "Select Computer");

    lv_obj_t *content = lv_win_get_content(win);
    lv_obj_set_style_pad_hor(content, 0, 0);

    fragment->grid_view = lv_gridview_create(content);
    lv_obj_set_size(fragment->grid_view, LV_PCT(100), LV_PCT(100));
    lv_obj_update_layout(fragment->grid_view);

    lv_obj_set_user_data(fragment->grid_view, fragment);
    lv_obj_set_style_pad_bottom(fragment->grid_view, LV_DPX(30), 0);
    lv_obj_set_style_pad_hor(fragment->grid_view, LV_DPX(30), 0);
    lv_obj_set_style_pad_gap(fragment->grid_view, LV_DPX(15), 0);
    lv_obj_set_style_pad_right(fragment->grid_view, LV_DPX(13), LV_PART_SCROLLBAR);
    lv_gridview_set_adapter(fragment->grid_view, &hosts_adapter);

    grid_size_populate(fragment);
    lv_obj_add_event_cb(fragment->grid_view, size_changed_cb, LV_EVENT_SIZE_CHANGED, fragment);
    lv_obj_add_event_cb(fragment->grid_view, host_item_clicked, LV_EVENT_CLICKED, fragment);
    lv_obj_add_event_cb(fragment->grid_view, grid_focused, LV_EVENT_FOCUSED, fragment);
    lv_obj_add_event_cb(fragment->grid_view, grid_unfocused, LV_EVENT_DEFOCUSED, fragment);
    lv_obj_add_event_cb(fragment->grid_view, grid_key_cb, LV_EVENT_KEY, fragment);

    return win;
}

static void obj_created(lv_fragment_t *self, lv_obj_t *obj) {
    LV_UNUSED(obj);
    hosts_fragment *fragment = (hosts_fragment *) self;
    host_manager_t *hosts_manager = fragment->app->host_manager;
    host_manager_register_listener(hosts_manager, &host_manager_listener, fragment);
    lv_gridview_set_data(fragment->grid_view, host_manager_get_hosts(hosts_manager));
    lv_group_t *group = app_ui_get_input_group(fragment->app->ui);
    if (group != NULL && lv_group_get_focused(group) == NULL) {
        hosts_fragment_focus_hosts(self);
    }

    host_manager_discovery_start(hosts_manager);
}

static void obj_will_delete(lv_fragment_t *self, lv_obj_t *obj) {
    LV_UNUSED(obj);
    hosts_fragment *fragment = (hosts_fragment *) self;
    host_manager_discovery_stop(fragment->app->host_manager);
}

static void obj_deleted(lv_fragment_t *self, lv_obj_t *obj) {
    LV_UNUSED(obj);
    hosts_fragment *fragment = (hosts_fragment *) self;
    host_manager_unregister_listener(fragment->app->host_manager, &host_manager_listener);
}

static void hosts_changed(array_list_t *list, host_manager_hosts_change change_type, int change_index, void *context) {
    hosts_fragment *fragment = (hosts_fragment *) context;
    lv_obj_t *grid = fragment->grid_view;
    switch (change_type) {
        case HOST_MANAGER_HOSTS_NEW: {
            lv_gridview_data_change_t changes[] = {
                    {.start = change_index, .remove_count = 0, .add_count = 1}
            };
            lv_gridview_set_data_advanced(grid, list, changes, 1);
            break;
        }
        case HOST_MANAGER_HOSTS_UPDATE: {
            lv_gridview_data_change_t changes[] = {
                    {.start = change_index, .remove_count = 1, .add_count = 1}
            };
            lv_gridview_set_data_advanced(grid, list, changes, 1);
            break;
        }
    }
}

static lv_obj_t *open_msgbox(hosts_fragment *fragment, const char *title, const char *message, const char *btns[]) {
    close_msgbox(fragment);
    lv_obj_t *mbox = lv_msgbox_create(NULL, title, message, btns, false);
    msgbox_fix_sizes(mbox, btns);
    lv_obj_add_event_cb(mbox, msgbox_del_cb, LV_EVENT_DELETE, fragment);
    lv_obj_center(mbox);
    return fragment->msgbox = mbox;
}

static void close_msgbox(hosts_fragment *fragment) {
    if (fragment->msgbox == NULL) {
        return;
    }
    lv_msgbox_close(fragment->msgbox);
    fragment->msgbox = NULL;
}

static void msgbox_confirm_cb(lv_event_t *e) {
    lv_obj_t *mbox = lv_event_get_current_target(e);
    lv_event_stop_processing(e);
    lv_msgbox_close(mbox);
}

static void msgbox_del_cb(lv_event_t *e) {
    hosts_fragment *fragment = lv_event_get_user_data(e);
    if (lv_event_get_current_target(e) != fragment->msgbox) {
        return;
    }
    fragment->msgbox = NULL;
}

static void authorization_cancel_cb(lv_event_t *e) {
    hosts_fragment *fragment = lv_event_get_user_data(e);
    lv_obj_t *mbox = lv_event_get_current_target(e);
    lv_event_stop_processing(e);
    lv_msgbox_close(mbox);
    host_manager_authorization_cancel(fragment->app->host_manager);
}

static int host_item_count(lv_obj_t *grid, void *data) {
    LV_UNUSED(grid);
    return array_list_size(data);
}

static lv_obj_t *host_item_create(lv_obj_t *grid) {
    hosts_fragment *fragment = lv_obj_get_user_data(grid);
    lv_obj_t *item_view = lv_btn_create(grid);
    lv_group_remove_obj(item_view);
    lv_obj_set_style_radius(item_view, 0, 0);
    lv_obj_set_layout(item_view, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(item_view, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(item_view, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    host_obj_holder *holder = malloc(sizeof(host_obj_holder));
    item_view->user_data = holder;

    holder->icon = lv_obj_create(item_view);
    lv_obj_remove_style_all(holder->icon);
    lv_obj_clear_flag(holder->icon, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_size(holder->icon, LV_DPX(128), LV_DPX(128));
    lv_obj_set_style_text_font(holder->icon, fragment->app->ui->iconfont.huge, 0);
    lv_obj_set_style_bg_color(holder->icon, lv_color_white(), 0);
    lv_obj_set_style_bg_opa(holder->icon, LV_OPA_30, 0);
    lv_obj_set_style_radius(holder->icon, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_img_src(holder->icon, BS_SYMBOL_DISPLAY, 0);

    holder->os_icon = lv_obj_create(holder->icon);
    lv_obj_remove_style_all(holder->os_icon);
    lv_obj_clear_flag(holder->os_icon, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_size(holder->os_icon, LV_DPX(40), LV_DPX(40));
    lv_obj_set_style_text_font(holder->os_icon, fragment->app->ui->iconfont.heading2, 0);
    lv_obj_align(holder->os_icon, LV_ALIGN_CENTER, 0, -LV_DPX(4));

    holder->name = lv_label_create(item_view);
    lv_obj_add_event_cb(item_view, host_item_delete, LV_EVENT_DELETE, NULL);
    lv_obj_set_size(item_view, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_add_flag(item_view, LV_OBJ_FLAG_EVENT_BUBBLE);
    return item_view;
}

static void host_item_delete(lv_event_t *e) {
    free(lv_event_get_current_target(e)->user_data);
}

static void host_item_bind(lv_obj_t *grid, lv_obj_t *item_view, void *data, int position) {
    LV_UNUSED(grid);
    host_obj_holder *holder = item_view->user_data;
    IHS_HostInfo *item = array_list_get(data, position);
    lv_label_set_text(holder->name, item->hostname);
    if (item->ostype >= IHS_SteamOSTypeWindows) {
        lv_obj_set_style_bg_img_src(holder->os_icon, BS_SYMBOL_WINDOWS, 0);
    } else if (item->ostype >= IHS_SteamOSTypeMacos && item->ostype < IHS_SteamOSTypeUnknown) {
        lv_obj_set_style_bg_img_src(holder->os_icon, BS_SYMBOL_APPLE, 0);
    } else {
        lv_obj_set_style_bg_img_src(holder->os_icon, LV_SYMBOL_DUMMY, 0);
    }
}

static void grid_size_populate(hosts_fragment *fragment) {
    lv_coord_t content_width = lv_obj_get_content_width(fragment->grid_view);
    int col_count = 5;
    if (content_width > 0) {
        lv_coord_t pad_column = lv_obj_get_style_pad_column(fragment->grid_view, 0);
        col_count = (content_width + pad_column) / (LV_DPX(150) + pad_column);
    }
    lv_gridview_set_config(fragment->grid_view, col_count, LV_DPX(200), LV_GRID_ALIGN_STRETCH,
                           LV_GRID_ALIGN_STRETCH);
}

static void host_item_clicked(lv_event_t *e) {
    hosts_fragment *fragment = e->user_data;
    lv_obj_t *target = lv_event_get_target(e);
    lv_obj_t *grid = fragment->grid_view;
    if (target->parent != grid) return;
    int index = lv_gridview_get_item_data_index(grid, target);
    if (index < 0) return;
    const IHS_HostInfo *item = array_list_get(lv_gridview_get_data(grid), index);
    launcher_fragment_set_selected_host(fragment->launcher_fragment, item->clientId);

    lv_fragment_manager_pop(lv_fragment_get_manager((lv_fragment_t *) fragment));
}

static void size_changed_cb(lv_event_t *e) {
    hosts_fragment *fragment = e->user_data;
    grid_size_populate(fragment);
}

static void grid_focused(lv_event_t *e) {
    if (e->target == e->current_target) {
        array_list_t *data = lv_gridview_get_data(e->target);
        if (data == NULL || array_list_size(data) == 0) {
            return;
        }
        lv_gridview_focus(e->target, 0);
    } else {
        int index = lv_gridview_get_focused_index(e->current_target);
        if (index >= 0) {
            lv_obj_t *item_view = lv_gridview_get_item_view(e->current_target, index);
            if (item_view != NULL && lv_obj_has_state(item_view, LV_STATE_FOCUSED)) {
                lv_obj_add_state(item_view, LV_STATE_FOCUS_KEY);
            }
        }
    }
}

static void grid_unfocused(lv_event_t *e) {
    if (e->target != e->current_target) return;
    lv_gridview_focus(e->target, -1);
}

static void grid_key_cb(lv_event_t *e) {
    if (e->target != e->current_target || e->stop_processing) return;
    switch (lv_event_get_key(e)) {
        case LV_KEY_UP: {
            lv_group_focus_prev(lv_group_get_default());
            break;
        }
    }
}

static bool event_cb(lv_fragment_t *self, int code, void *data) {
    (void) data;
    if (code == APP_UI_NAV_BACK) {
        lv_fragment_manager_pop(lv_fragment_get_manager(self));
        return true;
    }
    return false;
}

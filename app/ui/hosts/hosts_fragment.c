#include "hosts_fragment.h"

#include "app.h"
#include "backend/host_manager.h"
#include "lvgl/lv_gridview.h"
#include "ui/app_ui.h"
#include "ui/session/session.h"
#include "util/array_list.h"

typedef struct hosts_fragment {
    lv_fragment_t base;
    app_t *app;
    lv_coord_t col_dsc[3], row_dsc[7];
    lv_obj_t *grid_view;
} hosts_fragment;

typedef struct host_obj_holder {
    lv_obj_t *name;
    int position;
} host_obj_holder;

static void constructor(lv_fragment_t *self, void *arg);

static void destructor(lv_fragment_t *self);

static lv_obj_t *create_obj(lv_fragment_t *self, lv_obj_t *container);

static void obj_created(lv_fragment_t *self, lv_obj_t *obj);

static void obj_will_delete(lv_fragment_t *self, lv_obj_t *obj);

static void obj_deleted(lv_fragment_t *self, lv_obj_t *obj);

static void hosts_reloaded(array_list_t *list, void *context);

static int host_item_count(lv_obj_t *grid, void *data);

static int host_item_id(lv_obj_t *grid, void *data, int index);

static lv_obj_t *host_item_create(lv_obj_t *grid);

static void host_item_delete(lv_event_t *e);

static void host_item_clicked(lv_event_t *e);

static void grid_focused(lv_event_t *e);

static void grid_unfocused(lv_event_t *e);

static void host_item_bind(lv_obj_t *grid, lv_obj_t *item_view, void *data, int position);

const lv_fragment_class_t hosts_fragment_class = {
        .constructor_cb = constructor,
        .destructor_cb = destructor,
        .create_obj_cb = create_obj,
        .obj_created_cb = obj_created,
        .obj_will_delete_cb = obj_will_delete,
        .obj_deleted_cb = obj_deleted,
        .instance_size = sizeof(hosts_fragment),
};

static const host_manager_listener_t host_manager_listener = {
        .hosts_reloaded = hosts_reloaded,
};

static const lv_gridview_adapter_t hosts_adapter = {
        .item_count = host_item_count,
        .item_id = host_item_id,
        .create_view = host_item_create,
        .bind_view = host_item_bind,
};

static void constructor(lv_fragment_t *self, void *arg) {
    hosts_fragment *fragment = (hosts_fragment *) self;
    fragment->app = arg;
}

static void destructor(lv_fragment_t *self) {
    LV_UNUSED(self);
}

static lv_obj_t *create_obj(lv_fragment_t *self, lv_obj_t *container) {
    hosts_fragment *fragment = (hosts_fragment *) self;
    fragment->grid_view = lv_gridview_create(container);
    lv_obj_set_style_pad_all(fragment->grid_view, LV_DPX(10), 0);
    lv_gridview_set_adapter(fragment->grid_view, &hosts_adapter);
    lv_gridview_set_config(fragment->grid_view, 5, LV_DPX(200), LV_GRID_ALIGN_STRETCH, LV_GRID_ALIGN_STRETCH);
    lv_obj_add_event_cb(fragment->grid_view, host_item_clicked, LV_EVENT_CLICKED, fragment);
    lv_obj_add_event_cb(fragment->grid_view, grid_focused, LV_EVENT_FOCUSED, fragment);
    lv_obj_add_event_cb(fragment->grid_view, grid_unfocused, LV_EVENT_DEFOCUSED, fragment);
    return fragment->grid_view;
}

static void obj_created(lv_fragment_t *self, lv_obj_t *obj) {
    LV_UNUSED(obj);
    hosts_fragment *fragment = (hosts_fragment *) self;
    host_manager_t *hosts_manager = fragment->app->hosts_manager;
    host_manager_register_listener(hosts_manager, &host_manager_listener, fragment);
    host_manager_discovery_start(hosts_manager);
    lv_group_focus_obj(fragment->grid_view);
}

static void obj_will_delete(lv_fragment_t *self, lv_obj_t *obj) {
    LV_UNUSED(obj);
    hosts_fragment *fragment = (hosts_fragment *) self;
    host_manager_discovery_stop(fragment->app->hosts_manager);
}

static void obj_deleted(lv_fragment_t *self, lv_obj_t *obj) {
    LV_UNUSED(obj);
    hosts_fragment *fragment = (hosts_fragment *) self;
    host_manager_unregister_listener(fragment->app->hosts_manager, &host_manager_listener);
}

static void hosts_reloaded(array_list_t *list, void *context) {
    hosts_fragment *fragment = (hosts_fragment *) context;
    lv_obj_t *grid = fragment->grid_view;
    lv_gridview_set_data(grid, list);
}

static int host_item_count(lv_obj_t *grid, void *data) {
    LV_UNUSED(grid);
    return array_list_size(data);
}

static int host_item_id(lv_obj_t *grid, void *data, int index) {
    LV_UNUSED(grid);
    IHS_HostInfo *item = array_list_get(data, index);
    return (int) item->clientId;
}

static lv_obj_t *host_item_create(lv_obj_t *grid) {
    lv_obj_t *item_view = lv_btn_create(grid);
    lv_group_remove_obj(item_view);
    lv_obj_set_layout(item_view, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(item_view, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(item_view, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    host_obj_holder *holder = malloc(sizeof(host_obj_holder));
    item_view->user_data = holder;
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
    holder->position = position;
    IHS_HostInfo *item = array_list_get(data, position);
    lv_label_set_text(holder->name, item->hostname);
}

static void host_item_clicked(lv_event_t *e) {
    hosts_fragment *fragment = e->user_data;
    lv_obj_t *target = lv_event_get_target(e);
    lv_obj_t *grid = fragment->grid_view;
    if (target->parent != grid) return;
    host_obj_holder *holder = target->user_data;
    IHS_HostInfo *item = array_list_get(lv_gridview_get_data(grid), holder->position);
    app_ui_push_fragment(fragment->app->ui, &session_fragment_class, item);
}


static void grid_focused(lv_event_t *e) {
    if (e->target != e->current_target) return;
    array_list_t *data = lv_gridview_get_data(e->target);
    if (data == NULL || array_list_size(data) == 0) {
        return;
    }
    lv_gridview_focus(e->target, 0);
}

static void grid_unfocused(lv_event_t *e) {
    if (e->target != e->current_target) return;
    lv_gridview_focus(e->target, -1);
}
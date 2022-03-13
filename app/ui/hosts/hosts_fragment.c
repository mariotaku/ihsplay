#include "hosts_fragment.h"

#include "app.h"
#include "backend/hosts_manager.h"
#include "lvgl/lv_gridview.h"

typedef struct hosts_fragment {
    lv_fragment_t base;
    app_t *app;
    lv_coord_t col_dsc[3], row_dsc[7];
    lv_obj_t *grid_view;
} hosts_fragment;

static void constructor(lv_fragment_t *self, void *arg);

static lv_obj_t *create_obj(lv_fragment_t *self, lv_obj_t *container);

static void obj_created(lv_fragment_t *self, lv_obj_t *obj);

static void obj_will_delete(lv_fragment_t *self, lv_obj_t *obj);

static void obj_deleted(lv_fragment_t *self, lv_obj_t *obj);

static void hosts_reloaded(IHS_HostInfo *list, size_t length, void *context);

const lv_fragment_class_t hosts_fragment_class = {
        .constructor_cb = constructor,
        .create_obj_cb = create_obj,
        .obj_created_cb = obj_created,
        .obj_will_delete_cb = obj_will_delete,
        .obj_deleted_cb = obj_deleted,
        .instance_size = sizeof(hosts_fragment),
};

static const host_manager_listener_t host_manager_listener = {
        .hosts_reloaded = hosts_reloaded
};

static const lv_gridview_adapter_t hosts_adapter = {

};

static void constructor(lv_fragment_t *self, void *arg) {
    hosts_fragment *fragment = (hosts_fragment *) self;
    fragment->app = arg;
}

static lv_obj_t *create_obj(lv_fragment_t *self, lv_obj_t *container) {
    hosts_fragment *fragment = (hosts_fragment *) self;
    fragment->grid_view = lv_gridview_create(container);
    lv_gridview_set_adapter(fragment->grid_view, &hosts_adapter);
    lv_gridview_set_config(fragment->grid_view, LV_DPX(100), LV_DPX(100), LV_GRID_ALIGN_CENTER, LV_GRID_ALIGN_CENTER);
    return fragment->grid_view;
}

static void obj_created(lv_fragment_t *self, lv_obj_t *obj) {
    LV_UNUSED(obj);
    hosts_fragment *fragment = (hosts_fragment *) self;
    host_manager_t *hosts_manager = fragment->app->hosts_manager;
    host_manager_register_listener(hosts_manager, &host_manager_listener, fragment);
    host_manager_discovery_start(hosts_manager);
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

static void hosts_reloaded(IHS_HostInfo *list, size_t length, void *context) {
    hosts_fragment *fragment = (hosts_fragment *) context;
    lv_gridview_set_data(fragment->grid_view, );
}
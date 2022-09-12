#include "launcher.h"
#include "app.h"
#include "ui/hosts/hosts_fragment.h"
#include "ui/settings/settings.h"
#include "app_ui.h"

typedef struct app_root_fragment {
    lv_fragment_t base;
    app_t *app;
    lv_coord_t col_dsc[3], row_dsc[8];
    struct {
        lv_style_t root;
    } styles;
    lv_obj_t *nav_content;
} app_root_fragment;

static void constructor(lv_fragment_t *self, void *arg);

static void destructor(lv_fragment_t *self);

static lv_obj_t *create_obj(lv_fragment_t *self, lv_obj_t *container);

static lv_obj_t *nav_btn_create(lv_obj_t *container, const char *txt);

static void launcher_open(app_root_fragment *fragment, const lv_fragment_class_t *cls);

static void launcher_hosts(lv_event_t *e);

static void launcher_settings(lv_event_t *e);

static void launcher_quit(lv_event_t *e);

const lv_fragment_class_t launcher_fragment_class = {
        .constructor_cb = constructor,
        .destructor_cb = destructor,
        .create_obj_cb = create_obj,
        .instance_size = sizeof(app_root_fragment),
};

static void constructor(lv_fragment_t *self, void *arg) {
    app_ui_fragment_args_t *fargs = arg;
    app_root_fragment *fragment = (app_root_fragment *) self;
    fragment->app = fargs->app;
    fragment->col_dsc[0] = LV_DPX(200);
    fragment->col_dsc[1] = LV_GRID_FR(1);
    fragment->col_dsc[2] = LV_GRID_TEMPLATE_LAST;
    fragment->row_dsc[0] = LV_GRID_CONTENT;
    fragment->row_dsc[1] = LV_GRID_CONTENT;
    fragment->row_dsc[2] = LV_GRID_CONTENT;
    fragment->row_dsc[3] = LV_GRID_CONTENT;
    fragment->row_dsc[4] = LV_GRID_CONTENT;
    fragment->row_dsc[5] = LV_GRID_CONTENT;
    fragment->row_dsc[6] = LV_GRID_FR(1);
    fragment->row_dsc[7] = LV_GRID_TEMPLATE_LAST;

    lv_style_init(&fragment->styles.root);
    lv_style_set_pad_row(&fragment->styles.root, LV_DPX(10));
    lv_style_set_pad_column(&fragment->styles.root, LV_DPX(40));
    lv_style_set_pad_hor(&fragment->styles.root, LV_DPX(30));
    lv_style_set_pad_ver(&fragment->styles.root, LV_DPX(40));
    lv_style_set_bg_opa(&fragment->styles.root, LV_OPA_COVER);
    lv_style_set_bg_color(&fragment->styles.root, lv_color_black());
}

static void destructor(lv_fragment_t *self) {
    app_root_fragment *fragment = (app_root_fragment *) self;
    lv_style_reset(&fragment->styles.root);
}

static lv_obj_t *create_obj(lv_fragment_t *self, lv_obj_t *container) {
    app_root_fragment *fragment = (app_root_fragment *) self;
    lv_obj_t *root = lv_obj_create(container);
    lv_obj_add_style(root, &fragment->styles.root, 0);
    lv_obj_set_layout(root, LV_LAYOUT_GRID);
    lv_obj_set_grid_dsc_array(root, fragment->col_dsc, fragment->row_dsc);

    lv_obj_t *title = lv_label_create(root);
    lv_label_set_text(title, "Steam Link");
    lv_obj_set_size(title, LV_PCT(100), LV_SIZE_CONTENT);
    lv_obj_set_grid_cell(title, LV_GRID_ALIGN_START, 0, 2, LV_GRID_ALIGN_SPACE_AROUND, 0, 1);

    lv_obj_t *subtitle = lv_label_create(root);
    lv_label_set_text(subtitle, "Stream games from your computer with Steam");
    lv_obj_set_size(subtitle, LV_PCT(100), LV_SIZE_CONTENT);
    lv_obj_set_grid_cell(subtitle, LV_GRID_ALIGN_START, 0, 2, LV_GRID_ALIGN_SPACE_AROUND, 1, 1);

    lv_obj_t *btn_servers = nav_btn_create(root, "Start Playing");
    lv_obj_set_grid_cell(btn_servers, LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_SPACE_AROUND, 2, 1);
    lv_obj_add_event_cb(btn_servers, launcher_hosts, LV_EVENT_CLICKED, fragment);

    lv_obj_t *btn_settings = nav_btn_create(root, "Settings");
    lv_obj_set_grid_cell(btn_settings, LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_SPACE_AROUND, 3, 1);
    lv_obj_add_event_cb(btn_settings, launcher_settings, LV_EVENT_CLICKED, fragment);

    lv_obj_t *btn_support = nav_btn_create(root, "Support");
    lv_obj_set_grid_cell(btn_support, LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_SPACE_AROUND, 4, 1);

    lv_obj_t *btn_quit = nav_btn_create(root, "Quit");
    lv_obj_set_grid_cell(btn_quit, LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_SPACE_AROUND, 5, 1);
    lv_obj_add_event_cb(btn_quit, launcher_quit, LV_EVENT_CLICKED, fragment->app);

    lv_obj_t *nav_content = lv_obj_create(root);
    lv_obj_set_grid_cell(nav_content, LV_GRID_ALIGN_STRETCH, 1, 1, LV_GRID_ALIGN_STRETCH, 2, 5);
    fragment->nav_content = nav_content;

    lv_obj_set_size(root, LV_PCT(100), LV_PCT(100));

    launcher_open(fragment, &hosts_fragment_class);
    return root;
}

static lv_obj_t *nav_btn_create(lv_obj_t *container, const char *txt) {
    lv_obj_t *btn = lv_btn_create(container);
    lv_obj_t *label = lv_label_create(btn);
    lv_label_set_text(label, txt);
    return btn;
}

static void launcher_open(app_root_fragment *fragment, const lv_fragment_class_t *cls) {
    lv_fragment_manager_t *manager = ((lv_fragment_t *) fragment)->child_manager;
    lv_fragment_t *current = lv_fragment_manager_find_by_container(manager, fragment->nav_content);
    if (current != NULL && current->cls == cls) {
        return;
    }
    lv_fragment_t *f = lv_fragment_create(cls, fragment->app);
    lv_fragment_manager_replace(manager, f, &fragment->nav_content);
}

static void launcher_hosts(lv_event_t *e) {
    app_root_fragment *fragment = lv_event_get_user_data(e);
    launcher_open(fragment, &hosts_fragment_class);
}

static void launcher_settings(lv_event_t *e) {
    app_root_fragment *fragment = lv_event_get_user_data(e);
    launcher_open(fragment, &settings_fragment_class);
}

static void launcher_quit(lv_event_t *e) {
    app_quit(lv_event_get_user_data(e));
}

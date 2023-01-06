/**
 * @file lv_gridview.c
 *
 */

/*********************
 *      INCLUDES
 *********************/

#include "lv_gridview.h"
#include "src/core/lv_indev.h"
#include "src/extra/layouts/grid/lv_grid.h"

#if LV_USE_GRIDVIEW

/*********************
 *      DEFINES
 *********************/
#define MY_CLASS &lv_gridview_class

/**********************
 *      TYPEDEFS
 **********************/

typedef struct view_pool_ll_t {
    int position;
    lv_obj_t *item;
    struct view_pool_ll_t *prev, *next;
} view_pool_ll_t;

typedef struct lv_grid_t {
    lv_obj_t obj;
    lv_gridview_adapter_t adapter;
    /*Configs*/
    int column_count;
    lv_coord_t row_height;
    lv_grid_align_t column_align, row_align;

    void *data;
    /*States*/
    int item_count, row_count;
    int row_start, row_end;
    int focused_index, focus_when_available;
    /*Tmp data*/
    lv_coord_t *col_dsc, *row_dsc;
    lv_obj_t *placeholder;
    lv_coord_t content_height;
    lv_coord_t pad_row, pad_top, pad_bottom;

    /*Pending changes*/
    int old_item_count;
    const lv_gridview_data_change_t *changes;
    int num_changes;
    bool key_focus_clamp;

    view_pool_ll_t *pool_inuse, *pool_free;
    lv_style_t style_scrollbar, style_scrollbar_scrolled;
} lv_grid_t;

/**********************
 *  STATIC PROTOTYPES
 **********************/

static void update_col_dsc(lv_grid_t *grid);

static void update_row_dsc(lv_grid_t *grid, int row_count);

static void update_row_count(lv_grid_t *grid, int row_count);

static void scroll_cb(lv_event_t *event);

static void fill_rows(lv_grid_t *grid, int row_start, int row_end);

static void update_grid(lv_grid_t *grid);

static bool grid_recycle_item(lv_grid_t *grid, int position, bool optional);

static lv_obj_t *grid_obtain_item(lv_grid_t *grid, int position, bool *created);

static lv_obj_t *view_pool_take_by_position(view_pool_ll_t **pool, int position);

static bool view_pool_remove_by_instance(view_pool_ll_t **pool, const lv_obj_t *obj);

static view_pool_ll_t *view_pool_node_by_position(view_pool_ll_t *pool, int position);

static view_pool_ll_t *view_pool_node_by_instance(view_pool_ll_t *pool, const lv_obj_t *obj);

/**
 * Remove the node from view pool. Memory allocated by this node will not be freed.
 * @param head
 * @param cur
 * @return
 */
static view_pool_ll_t *view_pool_node_unlink(view_pool_ll_t *head, view_pool_ll_t *cur);

static lv_obj_t *view_pool_poll(view_pool_ll_t **pool);

static void view_pool_put(view_pool_ll_t **pool, int position, lv_obj_t *value);

static void view_pool_reset(view_pool_ll_t **pool);

static void lv_gridview_constructor(const lv_obj_class_t *class_p, lv_obj_t *obj);

static void lv_gridview_destructor(const lv_obj_class_t *class_p, lv_obj_t *obj);

static void lv_gridview_event(const lv_obj_class_t *class_p, lv_event_t *e);

static void key_cb(lv_grid_t *grid, lv_event_t *e);

static void cancel_cb(lv_grid_t *grid, lv_event_t *e);

static void press_cb(lv_grid_t *grid, lv_event_t *e);

static bool update_sizes(lv_grid_t *grid);

static void item_delete_cb(lv_event_t *event);

/**********************
 *  STATIC VARIABLES
 **********************/

const lv_obj_class_t lv_gridview_class = {
        .constructor_cb = lv_gridview_constructor,
        .destructor_cb = lv_gridview_destructor,
        .event_cb = lv_gridview_event,
        .width_def = LV_PCT(100),
        .height_def = LV_PCT(100),
        .editable = LV_OBJ_CLASS_EDITABLE_TRUE,
        .group_def = LV_OBJ_CLASS_GROUP_DEF_TRUE,
        .instance_size = (sizeof(lv_grid_t)),
        .base_class = &lv_obj_class,
};

/**********************
 *      MACROS
 **********************/
#define CEIL_DIVIDE(a, b) (((a) + (b) - 1) / (b))

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

lv_obj_t *lv_gridview_create(lv_obj_t *parent) {
    lv_obj_t *obj = lv_obj_class_create_obj(MY_CLASS, parent);
    lv_obj_class_init_obj(obj);
    return obj;
}

void lv_gridview_set_config(lv_obj_t *obj, int col_count, lv_coord_t row_height, lv_grid_align_t col_align,
                            lv_grid_align_t row_align) {
    lv_grid_t *grid = (lv_grid_t *) obj;
    bool column_count_changed = grid->column_count != col_count;
    if (column_count_changed) {
        // Scrap all views
        fill_rows(grid, 0, -1);
    }
    grid->column_count = col_count;
    grid->row_height = row_height;
    grid->column_align = col_align;
    grid->row_align = row_align;
    update_col_dsc(grid);
    if (column_count_changed) {
        int row_count = CEIL_DIVIDE(grid->item_count, grid->column_count);
        update_row_count(grid, row_count);
    }
    update_row_dsc(grid, grid->row_count);
    lv_obj_set_grid_dsc_array(obj, grid->col_dsc, grid->row_dsc);
    if (column_count_changed) {
        update_grid(grid);
    }
}

void lv_gridview_set_adapter(lv_obj_t *obj, const lv_gridview_adapter_t *adapter) {
    lv_memcpy(&((lv_grid_t *) obj)->adapter, adapter, sizeof(lv_gridview_adapter_t));
}

void lv_gridview_set_data(lv_obj_t *obj, void *data) {
    lv_gridview_set_data_advanced(obj, data, NULL, 0);
}

void lv_gridview_set_data_advanced(lv_obj_t *obj, void *data, const lv_gridview_data_change_t changes[],
                                   int num_changes) {
    lv_grid_t *grid = (lv_grid_t *) obj;
    lv_gridview_adapter_t adapter = grid->adapter;

    /* Backup item count */
    grid->old_item_count = grid->item_count;
    grid->changes = changes;
    grid->num_changes = num_changes;

    grid->data = data;
    bool count_changed = false;
    if (data) {
        int item_count = adapter.item_count(obj, data);
        count_changed = grid->item_count != item_count;
        grid->item_count = item_count;
        if (count_changed) {
            int row_count = CEIL_DIVIDE(item_count, grid->column_count);
            update_row_dsc(grid, row_count);
            lv_obj_set_grid_dsc_array(obj, grid->col_dsc, grid->row_dsc);
            update_row_count(grid, row_count);
        }
    }
    update_grid(grid);

    grid->changes = NULL;
    grid->num_changes = 0;
}

void *lv_gridview_get_data(lv_obj_t *obj) {
    lv_grid_t *grid = (lv_grid_t *) obj;
    return grid->data;
}

bool lv_gridview_focus(lv_obj_t *obj, int position) {
    lv_grid_t *grid = (lv_grid_t *) obj;
    bool has_focused = grid->focused_index >= 0 && position != grid->focused_index;
    if (has_focused) {
        view_pool_ll_t *node = view_pool_node_by_position(grid->pool_inuse, grid->focused_index);
        if (node) {
            lv_event_send(node->item, LV_EVENT_DEFOCUSED, lv_indev_get_act());
        }
        grid->focused_index = -1;
    }
    if (position < 0 || position >= grid->item_count) return false;
    view_pool_ll_t *node = view_pool_node_by_position(grid->pool_inuse, position);
    lv_obj_t *will_focus = node ? node->item : NULL;
    if (will_focus == NULL) {
        lv_obj_t *item = grid_obtain_item(grid, position, NULL);
        int row_idx = position / grid->column_count, col_idx = position % grid->column_count;
        lv_obj_set_grid_cell(item, grid->column_align, col_idx, 1, grid->row_align, row_idx, 1);
        grid->adapter.bind_view(&grid->obj, item, grid->data, position);
        will_focus = item;
    }
    if (!will_focus) return false;
    grid->focused_index = position;
    lv_event_send(will_focus, LV_EVENT_FOCUSED, lv_indev_get_act());
    return true;
}

bool lv_gridview_focus_when_available(lv_obj_t *obj, int position) {
    lv_grid_t *grid = (lv_grid_t *) obj;
    grid->focus_when_available = position;
    return true;
}

int lv_gridview_get_focused_index(lv_obj_t *obj) {
    lv_grid_t *grid = (lv_grid_t *) obj;
    return grid->focused_index;
}

void lv_gridview_set_key_focus_clamp(lv_obj_t *obj, bool enable) {
    lv_grid_t *grid = (lv_grid_t *) obj;
    grid->key_focus_clamp = enable;
}

void lv_gridview_rebind(lv_obj_t *obj) {
    lv_grid_t *grid = (lv_grid_t *) obj;
    for (int row_idx = LV_MAX(0, grid->row_start); row_idx <= grid->row_end; row_idx++) {
        for (int col_idx = 0; col_idx < grid->column_count; col_idx++) {
            int position = row_idx * grid->column_count + col_idx;
            if (position >= grid->item_count) continue;
            view_pool_ll_t *node = view_pool_node_by_position(grid->pool_inuse, position);
            if (!node) return;
            grid->adapter.bind_view(&grid->obj, node->item, grid->data, position);
        }
    }
}

void lv_gridview_rebind_item(lv_obj_t *obj, int position) {
    lv_grid_t *grid = (lv_grid_t *) obj;
    view_pool_ll_t *node = view_pool_node_by_position(grid->pool_inuse, position);
    if (!node) return;
    grid->adapter.bind_view(&grid->obj, node->item, grid->data, position);
}

int lv_gridview_get_item_data_index(lv_obj_t *obj, lv_obj_t *item_view) {
    lv_grid_t *grid = (lv_grid_t *) obj;
    view_pool_ll_t *node = view_pool_node_by_instance(grid->pool_inuse, item_view);
    if (!node) return -1;
    return node->position;
}

lv_obj_t *lv_gridview_get_item_view(lv_obj_t *obj, int position) {
    lv_grid_t *grid = (lv_grid_t *) obj;
    view_pool_ll_t *node = view_pool_node_by_position(grid->pool_inuse, position);
    if (!node) return NULL;
    return node->item;
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

static void lv_gridview_constructor(const lv_obj_class_t *class_p, lv_obj_t *obj) {
    LV_UNUSED(class_p);
    lv_grid_t *grid = (lv_grid_t *) obj;
    lv_obj_set_layout(obj, LV_LAYOUT_GRID);

    lv_gridview_set_config(obj, 1, 1, LV_GRID_ALIGN_STRETCH, LV_GRID_ALIGN_STRETCH);

    lv_obj_t *placeholder = lv_obj_create(obj);

    // Here we steal scroll bar style for parent
    lv_style_init(&grid->style_scrollbar);
    lv_style_set_bg_color(&grid->style_scrollbar, lv_obj_get_style_bg_color(placeholder, LV_PART_SCROLLBAR));
    lv_style_set_radius(&grid->style_scrollbar, lv_obj_get_style_radius(placeholder, LV_PART_SCROLLBAR));
    lv_style_set_pad_right(&grid->style_scrollbar, lv_obj_get_style_pad_right(placeholder, LV_PART_SCROLLBAR));
    lv_style_set_pad_top(&grid->style_scrollbar, lv_obj_get_style_pad_top(placeholder, LV_PART_SCROLLBAR));
    lv_style_set_width(&grid->style_scrollbar, lv_obj_get_style_width(placeholder, LV_PART_SCROLLBAR));
    lv_style_set_height(&grid->style_scrollbar, lv_obj_get_style_height(placeholder, LV_PART_SCROLLBAR));
    lv_style_set_bg_opa(&grid->style_scrollbar, lv_obj_get_style_bg_opa(placeholder, LV_PART_SCROLLBAR));
    lv_style_set_transition(&grid->style_scrollbar, lv_obj_get_style_transition(placeholder, LV_PART_SCROLLBAR));

    lv_style_init(&grid->style_scrollbar_scrolled);
    lv_style_set_bg_opa(&grid->style_scrollbar_scrolled, LV_OPA_COVER);

    lv_obj_add_style(obj, &grid->style_scrollbar, LV_PART_SCROLLBAR);
    lv_obj_add_style(obj, &grid->style_scrollbar_scrolled, LV_PART_SCROLLBAR | LV_STATE_SCROLLED);

    lv_obj_remove_style_all(placeholder);
    grid->placeholder = placeholder;
    grid->row_start = -1;
    grid->row_end = -1;
    grid->focused_index = -1;
    grid->focus_when_available = -1;
}

static void lv_gridview_destructor(const lv_obj_class_t *class_p, lv_obj_t *obj) {
    LV_UNUSED(class_p);
    lv_grid_t *grid = (lv_grid_t *) obj;
    view_pool_reset(&grid->pool_free);
    view_pool_reset(&grid->pool_inuse);
    if (grid->row_dsc) {
        lv_mem_free(grid->row_dsc);
    }
    lv_mem_free(grid->col_dsc);
    lv_style_reset(&grid->style_scrollbar);
    lv_style_reset(&grid->style_scrollbar_scrolled);
}

static void lv_gridview_event(const lv_obj_class_t *class_p, lv_event_t *e) {
    LV_UNUSED(class_p);

    /*Call the ancestor's event handler*/
    lv_res_t res;

    res = lv_obj_event_base(MY_CLASS, e);
    if (res != LV_RES_OK) return;
    lv_grid_t *grid = (lv_grid_t *) e->current_target;
    if (e->target != e->current_target) return;

    switch (e->code) {
        case LV_EVENT_SCROLL:
            scroll_cb(e);
            break;
        case LV_EVENT_KEY:
            key_cb(grid, e);
            break;
        case LV_EVENT_CANCEL:
            cancel_cb(grid, e);
            break;
        case LV_EVENT_PRESSED:
        case LV_EVENT_PRESSING:
        case LV_EVENT_RELEASED:
        case LV_EVENT_CLICKED:
        case LV_EVENT_PRESS_LOST:
        case LV_EVENT_SHORT_CLICKED:
        case LV_EVENT_LONG_PRESSED:
        case LV_EVENT_LONG_PRESSED_REPEAT:
            press_cb(grid, e);
            break;
        case LV_EVENT_SIZE_CHANGED:
        case LV_EVENT_STYLE_CHANGED:
            if (update_sizes(grid)) {
                update_grid(grid);
            }
            break;
        default:
            return;
    }
}

static void update_col_dsc(lv_grid_t *grid) {
    int column_count = grid->column_count;
    lv_coord_t *col_dsc = lv_mem_realloc(grid->col_dsc, (column_count + 1) * sizeof(lv_coord_t));
    for (int i = 0; i < column_count; i++) {
        col_dsc[i] = LV_GRID_FR(1);
    }
    col_dsc[column_count] = LV_GRID_TEMPLATE_LAST;
    grid->col_dsc = col_dsc;
}

static void update_row_dsc(lv_grid_t *grid, int row_count) {
    if (row_count == 0) {
        row_count = 1;
    }
    LV_ASSERT(row_count > 0);
    int array_size = row_count + 1;
    lv_coord_t *row_dsc = lv_mem_realloc(grid->row_dsc, array_size * sizeof(lv_coord_t));
    for (int i = 0; i < row_count; i++) {
        row_dsc[i] = grid->row_height;
    }
    row_dsc[array_size - 1] = LV_GRID_TEMPLATE_LAST;
    grid->row_dsc = row_dsc;
}

static void update_row_count(lv_grid_t *grid, int row_count) {
    if (row_count <= 0) return;
    lv_obj_set_grid_cell(grid->placeholder, grid->column_align, 0, 1, grid->row_align, row_count - 1, 1);
    grid->row_count = row_count;
}

static void scroll_cb(lv_event_t *event) {
    update_grid((lv_grid_t *) lv_event_get_current_target(event));
}

static void key_cb(lv_grid_t *grid, lv_event_t *e) {
    int offset;
    switch (lv_event_get_key(e)) {
        case LV_KEY_LEFT:
            offset = -1;
            break;
        case LV_KEY_RIGHT:
            offset = 1;
            break;
        case LV_KEY_UP:
            offset = -grid->column_count;
            break;
        case LV_KEY_DOWN:
            offset = grid->column_count;
            break;
        default:
            return;
    }
    int index = grid->focused_index;
    if (index < 0) {
        index = grid->row_start * grid->column_count;
    } else {
        index += offset;
    }
    int focus_position = index;
    if (grid->key_focus_clamp) {
        focus_position = LV_CLAMP(0, index, grid->item_count - 1);
    } else if (index < 0 || index >= grid->item_count) {
        return;
    }
    if (lv_gridview_focus((lv_obj_t *) grid, focus_position)) {
        lv_event_stop_processing(e);
    }
}

static void cancel_cb(lv_grid_t *grid, lv_event_t *e) {
    LV_UNUSED(e);
    lv_gridview_focus((lv_obj_t *) grid, -1);
}

static void press_cb(lv_grid_t *grid, lv_event_t *e) {
    if (e->current_target != e->target) return;
    lv_indev_t *indev = lv_indev_get_act();
    if (lv_indev_get_type(indev) != LV_INDEV_TYPE_KEYPAD) {
        return;
    }
    int index = grid->focused_index;
    if (index < 0) return;
    view_pool_ll_t *node = view_pool_node_by_position(grid->pool_inuse, index);
    if (!node) return;
    lv_event_send(node->item, e->code, indev);
}


/**
 * Cache paddings and sizes for performance
 * @param grid
 */
static bool update_sizes(lv_grid_t *grid) {
    lv_obj_t *obj = &grid->obj;
    bool changed = false;
    lv_coord_t content_height = lv_obj_get_content_height(obj);
    if (grid->content_height != content_height) {
        grid->content_height = content_height;
        changed = true;
    }
    lv_coord_t pad_row = lv_obj_get_style_pad_row(obj, 0);
    if (grid->pad_row != pad_row) {
        grid->pad_row = pad_row;
        changed = true;
    }
    lv_coord_t pad_top = lv_obj_get_style_pad_top(obj, 0);
    if (grid->pad_top != pad_top) {
        grid->pad_top = pad_top;
        changed = true;
    }
    lv_coord_t pad_bottom = lv_obj_get_style_pad_bottom(obj, 0);
    if (grid->pad_bottom != pad_bottom) {
        grid->pad_bottom = pad_bottom;
        changed = true;
    }
    return changed;
}

static void update_grid(lv_grid_t *grid) {
    lv_obj_t *obj = &grid->obj;
    int content_height = grid->content_height;
    if (content_height <= 0) return;
    lv_coord_t scroll_y = lv_obj_get_scroll_y(obj);
    lv_coord_t pad_row = grid->pad_row;
    lv_coord_t pad_top = grid->pad_top;
    lv_coord_t pad_bottom = grid->pad_bottom;
    lv_coord_t extend = grid->row_height / 4;
    lv_coord_t row_height = grid->row_height;
    /** Beginning index of rows to render */
    int render_row_start = -1;
    for (int i = 0; i < grid->row_count; i++) {
        lv_coord_t row_top = row_height * i + pad_row * i;
        lv_coord_t row_bottom = row_top + row_height;
        lv_coord_t start_bound = scroll_y - pad_top - extend;
        if (start_bound <= row_bottom) {
            render_row_start = i;
            break;
        }
    }
    /** Ending row index to render */
    int render_row_end = -1;
    for (int i = grid->row_count - 1; i >= render_row_start; i--) {
        lv_coord_t row_top = row_height * i + pad_row * i;
        lv_coord_t end_bound = scroll_y + content_height + pad_bottom + extend;
        if (end_bound >= row_top) {
            render_row_end = i;
            break;
        }
    }
    /** Ending visible row index (can be greater than actual) */
    int visible_row_end = render_row_start + CEIL_DIVIDE((content_height + pad_row + extend), row_height + pad_row);
    /** Beginning item index to render */
    int render_item_start = grid->column_count * render_row_start;
    /** Inclusive ending index could possibly visible */
    int max_item_end = grid->column_count * (visible_row_end + 1) - 1;

    int expect_item_count = grid->old_item_count;
    if (grid->num_changes <= 0) {
        expect_item_count = grid->item_count;
    }

    for (int change_index = 0; change_index < grid->num_changes; change_index++) {
        /* 1. Recycle removed items in change info */
        lv_gridview_data_change_t change = grid->changes[change_index];
        LV_ASSERT(change.start >= 0);
        int del_from = change.start;
        for (int del_pos = del_from; del_pos < del_from + change.remove_count; del_pos++) {
            grid_recycle_item(grid, del_pos, true);
        }
        /* 2. Move existing items to new positions */
        int move_from = change.start + change.remove_count, move_delta = change.add_count - change.remove_count;
        if (move_delta != 0) {
            for (int old_pos = LV_MIN(expect_item_count - 1, max_item_end); old_pos >= move_from; old_pos--) {
                int new_pos = old_pos + move_delta;
                if (new_pos < render_item_start || new_pos > max_item_end) {
                    grid_recycle_item(grid, old_pos, true);
                    continue;
                }
                view_pool_ll_t *node = view_pool_node_by_position(grid->pool_inuse, old_pos);
                if (node == NULL) {
                    continue;
                }
                uint8_t row_idx = new_pos / grid->column_count, col_idx = new_pos % grid->column_count;
                node->position = new_pos;
                lv_obj_set_grid_cell(node->item, grid->column_align, col_idx, 1, grid->row_align, row_idx, 1);
            }
        }
        expect_item_count += (change.add_count - change.remove_count);
    }
    LV_ASSERT(expect_item_count == grid->item_count);
    // Fill cells and recycle excess items
    fill_rows(grid, render_row_start, render_row_end);
}

static void fill_rows(lv_grid_t *grid, int row_start, int row_end) {
    // Mark unused ranges (top + bottom)
    int old_start = grid->row_start, old_end = grid->row_end;
    // Put excess items to recycler
    for (int row_idx = LV_MAX(0, old_start); row_idx < row_start; row_idx++) {
        for (int col_idx = 0; col_idx < grid->column_count; col_idx++) {
            int position = row_idx * grid->column_count + col_idx;
            if (position >= grid->item_count) continue;
            grid_recycle_item(grid, position, false);
        }
    }
    for (int row_idx = LV_MAX(0, row_end + 1); row_idx <= old_end; row_idx++) {
        for (int col_idx = 0; col_idx < grid->column_count; col_idx++) {
            int position = row_idx * grid->column_count + col_idx;
            if (position >= grid->item_count) continue;
            grid_recycle_item(grid, position, false);
        }
    }
    if (row_start < 0 || row_end < row_start) {
        return;
    }
    // Refresh
    for (int row_idx = row_start; row_idx <= row_end; row_idx++) {
        for (int col_idx = 0; col_idx < grid->column_count; col_idx++) {
            int position = row_idx * grid->column_count + col_idx;
            if (position >= grid->item_count) break;
            bool created = false;
            lv_obj_t *item = grid_obtain_item(grid, position, &created);
            if (created) {
                lv_obj_set_grid_cell(item, grid->column_align, col_idx, 1, grid->row_align, row_idx, 1);
                grid->adapter.bind_view(&grid->obj, item, grid->data, position);
            }
        }
    }
    grid->row_start = row_start;
    grid->row_end = row_end;
    if (grid->focus_when_available >= 0 && grid->focus_when_available < grid->item_count) {
        lv_gridview_focus((lv_obj_t *) grid, grid->focus_when_available);
        grid->focus_when_available = -1;
    }
}

static bool grid_recycle_item(lv_grid_t *grid, int position, bool optional) {
    // Move item from inuse pool to free pool
    lv_obj_t *item = view_pool_take_by_position(&grid->pool_inuse, position);
    if (optional && item == NULL) {
        return false;
    }
    LV_ASSERT_MSG(item, "should not recycle invalid item");
    lv_obj_add_flag(item, LV_OBJ_FLAG_HIDDEN);
    view_pool_put(&grid->pool_free, -1, item);
    return true;
}

static lv_obj_t *grid_obtain_item(lv_grid_t *grid, int position, bool *created) {
    // Move item from free pool to inuse pool, or create one
    view_pool_ll_t *cur = view_pool_node_by_position(grid->pool_inuse, position);
    lv_obj_t *item = cur ? cur->item : NULL;
    if (item) {
        if (created != NULL) {
            *created = false;
        }
        return item;
    }
    item = view_pool_poll(&grid->pool_free);
    if (!item) {
        item = grid->adapter.create_view((lv_obj_t *) grid);
        lv_obj_add_event_cb(item, item_delete_cb, LV_EVENT_DELETE, grid);
    }
    lv_obj_clear_flag(item, LV_OBJ_FLAG_HIDDEN);
    view_pool_put(&grid->pool_inuse, position, item);
    if (created != NULL) {
        *created = true;
    }
    return item;
}

static lv_obj_t *view_pool_take_by_position(view_pool_ll_t **pool, int position) {
    view_pool_ll_t *node = view_pool_node_by_position(*pool, position);
    if (!node) return NULL;
    lv_obj_t *item = node->item;
    (*pool) = view_pool_node_unlink(*pool, node);
    lv_mem_free(node);
    return item;
}

static bool view_pool_remove_by_instance(view_pool_ll_t **pool, const lv_obj_t *obj) {
    view_pool_ll_t *node = view_pool_node_by_instance(*pool, obj);
    if (!node) return false;
    (*pool) = view_pool_node_unlink(*pool, node);
    lv_mem_free(node);
    return true;
}

static view_pool_ll_t *view_pool_node_by_position(view_pool_ll_t *pool, int position) {
    for (view_pool_ll_t *cur = pool; cur != NULL; cur = cur->next) {
        if (cur->position == position) {
            return cur;
        }
    }
    return NULL;
}

static view_pool_ll_t *view_pool_node_by_instance(view_pool_ll_t *pool, const lv_obj_t *obj) {
    for (view_pool_ll_t *cur = pool; cur != NULL; cur = cur->next) {
        if (cur->item == obj) {
            return cur;
        }
    }
    return NULL;
}

static view_pool_ll_t *view_pool_node_unlink(view_pool_ll_t *head, view_pool_ll_t *cur) {
    LV_ASSERT_NULL(head);
    LV_ASSERT_NULL(cur);
    view_pool_ll_t *prev = cur->prev;
    if (prev) {
        prev->next = cur->next;
    } else {
        head = cur->next;
    }
    if (cur->next) {
        cur->next->prev = prev;
    }
    return head;
}

static lv_obj_t *view_pool_poll(view_pool_ll_t **pool) {
    view_pool_ll_t *head = *pool;
    if (!head) return NULL;
    lv_obj_t *item = head->item;
    if (head->next) {
        head->next->prev = NULL;
    }
    *pool = head->next;
    lv_mem_free(head);
    return item;
}

static void view_pool_put(view_pool_ll_t **pool, int position, lv_obj_t *value) {
    view_pool_ll_t *head = lv_mem_alloc(sizeof(view_pool_ll_t));
    lv_memset_00(head, sizeof(view_pool_ll_t));
    head->item = value;
    head->position = position;
    view_pool_ll_t *old_head = *pool;
    head->next = old_head;
    if (old_head) {
        old_head->prev = head;
    }
    *pool = head;
}

static void view_pool_reset(view_pool_ll_t **pool) {
    view_pool_ll_t *cur = *pool;
    while (cur) {
        view_pool_ll_t *tmp = cur;
        cur = cur->next;
        lv_mem_free(tmp);
    }
    *pool = NULL;
}

static void item_delete_cb(lv_event_t *event) {
    lv_grid_t *grid = lv_event_get_user_data(event);
    view_pool_remove_by_instance(&grid->pool_inuse, lv_event_get_current_target(event));
    view_pool_remove_by_instance(&grid->pool_free, lv_event_get_current_target(event));
}

#endif /* LV_USE_GRIDVIEW */
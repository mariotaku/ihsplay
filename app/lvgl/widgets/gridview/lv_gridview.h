/**
 * @file lv_gridview.h
 *
 */

#ifndef LV_GRIDVIEW_H
#define LV_GRIDVIEW_H

#ifdef __cplusplus
extern "C" {
#endif

#include "src/lv_conf_internal.h"

#if LV_USE_GRIDVIEW
#if LV_USE_GRID == 0
#error "To use gridview, grid layout must be enabled"
#endif

#include "src/core/lv_obj.h"
#include "src/extra/layouts/grid/lv_grid.h"

/*********************
 *      INCLUDES
 *********************/

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/
extern const lv_obj_class_t lv_gridview_class;

typedef struct _lv_gridview_adapter_t {
    int (*item_count)(lv_obj_t *, void *data);

    lv_obj_t *(*create_view)(lv_obj_t *);

    void (*bind_view)(lv_obj_t *, lv_obj_t *item_view, void *data, int position);

    int (*item_id)(lv_obj_t *, void *data, int position);
} lv_gridview_adapter_t;

/**
 * This data change struct holds information similar to the arguments of array.splice() in JavaScript.
 */
typedef struct _lv_gridview_data_change_t {
    /**
     * Zero-based index at which to start changing the data.
     */
    int start;
    /**
     * The number of items in the data to remove from start.
     */
    int remove_count;
    /**
     * The number of items to add to the data, beginning from start.
     */
    int add_count;
} lv_gridview_data_change_t;

/**********************
 * GLOBAL PROTOTYPES
 **********************/

lv_obj_t *lv_gridview_create(lv_obj_t *parent);

void lv_gridview_set_config(lv_obj_t *obj, int col_count, lv_coord_t row_height, lv_grid_align_t col_align,
                            lv_grid_align_t row_align);

void lv_gridview_set_adapter(lv_obj_t *obj, const lv_gridview_adapter_t *adapter);

/**
 *
 * @param obj
 * @param data
 * @param changes Can be NULL, which will invalidate the whole dataset.
 * @param num_changes Number of changes, non-positive number will invalidate the whole dataset.
 */
void lv_gridview_set_data(lv_obj_t *obj, void *data, const lv_gridview_data_change_t changes[], int num_changes);

void *lv_gridview_get_data(lv_obj_t *obj);

void lv_gridview_focus(lv_obj_t *obj, int position);

int lv_gridview_get_focused_index(lv_obj_t *obj);

void lv_gridview_rebind(lv_obj_t *obj);

void lv_gridview_rebind_item(lv_obj_t *obj, int position);
/**********************
 *      MACROS
 **********************/

#endif /*LV_USE_GRIDVIEW*/

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_GRIDVIEW_H*/

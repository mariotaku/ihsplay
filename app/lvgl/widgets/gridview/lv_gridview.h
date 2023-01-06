/**
 * @file lv_gridview.h
 * @brief A widget for displaying large dataset as a grid with reusable item views
 *
 * Grid view is a concept similar to GridView or RecyclerView in Android, or UICollectionView in iOS.
 * It recycles item views when they are scrolling out of visible viewport, and reuse them when needed.
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
    /**
     * Get item count of the data set
     * @param grid Grid view object
     * @param data Data set of the grid
     * @return Item count of the dataset
     */
    int (*item_count)(lv_obj_t *grid, void *data);

    /**
     * Create the item view
     * @param grid Grid view object
     * @return
     */
    lv_obj_t *(*create_view)(lv_obj_t *grid);

    /**
     * Update the item view with data and position
     * @param grid Grid view object
     * @param item_view Item view for binding
     * @param data Data set of the grid
     * @param position Position of the item in the data set
     */
    void (*bind_view)(lv_obj_t *grid, lv_obj_t *item_view, void *data, int position);
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
/**
 * Create a grid view object
 * @param parent pointer to an object where the new object should be created
 * @return  pointer of the grid view object
 */
lv_obj_t *lv_gridview_create(lv_obj_t *parent);

/**
 * Configure the grid view object
 * @param obj Grid view object
 * @param col_count Number of columns
 * @param row_height
 * @param col_align
 * @param row_align
 */
void lv_gridview_set_config(lv_obj_t *obj, int col_count, lv_coord_t row_height, lv_grid_align_t col_align,
                            lv_grid_align_t row_align);

/**
 * Set adapter for the grid view.
 * @param obj
 * @param adapter
 */
void lv_gridview_set_adapter(lv_obj_t *obj, const lv_gridview_adapter_t *adapter);

/**
 * Update data of the grid view. It will invalidate the whole dataset, which makes all the visible items to be recycled.
 * @param obj Grid view object
 * @param data Data set for update
 */
void lv_gridview_set_data(lv_obj_t *obj, void *data);

/**
 * Update data of the grid view. And specify what has changed in the dataset.
 * @param obj Grid view object
 * @param data Data set for update
 * @param changes Can be NULL, which will invalidate the whole dataset.
 * @param num_changes Number of changes, non-positive number will invalidate the whole dataset.
 */
void lv_gridview_set_data_advanced(lv_obj_t *obj, void *data, const lv_gridview_data_change_t changes[],
                                   int num_changes);

/**
 * Get the data set was previously set for the grid view
 * @param obj Grid view object
 * @return Data set of this grid view
 */
void *lv_gridview_get_data(lv_obj_t *obj);

bool lv_gridview_focus(lv_obj_t *obj, int position);

bool lv_gridview_focus_when_available(lv_obj_t *obj, int position);

int lv_gridview_get_focused_index(lv_obj_t *obj);

/**
 * When focusing first and last row, up/down key will make the grid view to focus on first/last item.
 * @param obj Grid view object
 * @param enable true to enable this feature
 */
void lv_gridview_set_key_focus_clamp(lv_obj_t *obj, bool enable);

/**
 * Ask adapter to bind all the visible items again
 * @param obj
 */
void lv_gridview_rebind(lv_obj_t *obj);

/**
 * Ask adapter to bind specified item again. It has no effect for an invisible item.
 * @param obj
 * @param position
 */
void lv_gridview_rebind_item(lv_obj_t *obj, int position);

/**
 * Get data set index of an item view
 * @param grid
 * @param item_view
 * @return
 */
int lv_gridview_get_item_data_index(lv_obj_t *obj, lv_obj_t *item_view);

lv_obj_t *lv_gridview_get_item_view(lv_obj_t *obj, int position);

/**********************
 *      MACROS
 **********************/

#endif /*LV_USE_GRIDVIEW*/

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_GRIDVIEW_H*/

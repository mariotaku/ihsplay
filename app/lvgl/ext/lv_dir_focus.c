#include "lv_dir_focus.h"

static lv_style_prop_t props[4] = {
        LV_STYLE_PROP_INV,
        LV_STYLE_PROP_INV,
        LV_STYLE_PROP_INV,
        LV_STYLE_PROP_INV
};

typedef enum focus_dir_index {
    DIR_INDEX_LEFT = 0,
    DIR_INDEX_RIGHT = 1,
    DIR_INDEX_TOP = 2,
    DIR_INDEX_BOTTOM = 3,
} focus_dir_index;

void lv_dir_focus_register() {
    LV_ASSERT(lv_is_initialized());
    props[DIR_INDEX_LEFT] = lv_style_register_prop(LV_STYLE_PROP_FLAG_NONE);
    props[DIR_INDEX_RIGHT] = lv_style_register_prop(LV_STYLE_PROP_FLAG_NONE);
    props[DIR_INDEX_TOP] = lv_style_register_prop(LV_STYLE_PROP_FLAG_NONE);
    props[DIR_INDEX_BOTTOM] = lv_style_register_prop(LV_STYLE_PROP_FLAG_NONE);
}

void lv_obj_set_dir_focus_obj(lv_obj_t *obj, lv_dir_t dir, const lv_obj_t *to_focus) {
    lv_style_value_t value = {.ptr = to_focus};
    if (dir & LV_DIR_LEFT) {
        lv_obj_set_local_style_prop(obj, props[DIR_INDEX_LEFT], value, 0);
    }
    if (dir & LV_DIR_RIGHT) {
        lv_obj_set_local_style_prop(obj, props[DIR_INDEX_RIGHT], value, 0);
    }
    if (dir & LV_DIR_TOP) {
        lv_obj_set_local_style_prop(obj, props[DIR_INDEX_TOP], value, 0);
    }
    if (dir & LV_DIR_BOTTOM) {
        lv_obj_set_local_style_prop(obj, props[DIR_INDEX_BOTTOM], value, 0);
    }
}

bool lv_obj_focus_dir(lv_obj_t *obj, lv_dir_t dir) {
    lv_style_value_t value;
    switch (dir) {
        case LV_DIR_LEFT:
            if (lv_obj_get_local_style_prop(obj, props[DIR_INDEX_LEFT], &value, 0) != LV_RES_OK) {
                return false;
            }
            break;
        case LV_DIR_RIGHT:
            if (lv_obj_get_local_style_prop(obj, props[DIR_INDEX_RIGHT], &value, 0) != LV_RES_OK) {
                return false;
            }
            break;
        case LV_DIR_TOP:
            if (lv_obj_get_local_style_prop(obj, props[DIR_INDEX_TOP], &value, 0) != LV_RES_OK) {
                return false;
            }
            break;
        case LV_DIR_BOTTOM:
            if (lv_obj_get_local_style_prop(obj, props[DIR_INDEX_BOTTOM], &value, 0) != LV_RES_OK) {
                return false;
            }
            break;
        default:
            return false;
    }
    lv_group_t *group = lv_obj_get_group(obj);
    lv_obj_t **group_item = NULL;
    _LV_LL_READ(&group->obj_ll, group_item) {
        if (*group_item == value.ptr) {
            break;
        }
    }
    if (group_item == NULL) {
        return false;
    }
    lv_group_focus_obj(*group_item);
    return true;
}

bool lv_obj_focus_dir_by_key(lv_obj_t *obj, lv_key_t dir) {
    switch (dir) {
        case LV_KEY_LEFT: {
            return lv_obj_focus_dir(obj, LV_DIR_LEFT);
        }
        case LV_KEY_RIGHT: {
            return lv_obj_focus_dir(obj, LV_DIR_RIGHT);
        }
        case LV_KEY_UP: {
            return lv_obj_focus_dir(obj, LV_DIR_TOP);
        }
        case LV_KEY_DOWN: {
            return lv_obj_focus_dir(obj, LV_DIR_BOTTOM);
        }
        default: {
            return false;
        }
    }
}


#include "group_utils.h"

lv_obj_t *ui_group_first_in_parent(lv_group_t *group, lv_obj_t *parent) {
    lv_obj_t **obj = NULL;
    _LV_LL_READ(&group->obj_ll, obj) {
        lv_obj_t *cur = *obj;
        if (cur == parent) {
            return cur;
        }
        while (cur != NULL) {
            cur = lv_obj_get_parent(cur);
            if (cur == parent) {
                return *obj;
            }
        }
    }
    return NULL;
}
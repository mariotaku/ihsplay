#include "msgbox_ext.h"
#include "ui/app_ui.h"
#include "lv_child_group.h"

static void msgbox_key(lv_event_t *event);

static void msgbox_cancel(lv_event_t *event);

static void msgbox_destroy(lv_event_t *event);

void msgbox_inject_nav(app_ui_t *ui, lv_obj_t *obj) {
    lv_group_t *group = lv_group_create();
    group->user_data = ui;
    app_ui_push_modal_group(ui, group);
    lv_obj_add_event_cb(obj, cb_child_group_add, LV_EVENT_CHILD_CREATED, group);
    lv_obj_add_event_cb(obj, msgbox_key, LV_EVENT_KEY, NULL);
    lv_obj_add_event_cb(obj, msgbox_cancel, LV_EVENT_CANCEL, NULL);
    lv_obj_add_event_cb(obj, msgbox_destroy, LV_EVENT_DELETE, group);
}

void msgbox_fix_sizes(lv_obj_t *obj, const char *btn_texts[]) {
    lv_obj_t *btns = lv_msgbox_get_btns(obj);
    if (btns != NULL) {
        int i;
        for (i = 0; btn_texts[i][0] != '\0'; i++);
        lv_obj_set_width(btns, i * LV_DPX(70) + (i - 1) * LV_DPX(10));
    }
}

static void msgbox_key(lv_event_t *event) {
    lv_obj_t *target = lv_event_get_target(event);
    lv_group_t *group = lv_obj_get_group(target);
    if (!group) return;
    switch (lv_event_get_key(event)) {
        case LV_KEY_UP: {
            lv_group_focus_prev(group);
            break;
        }
        case LV_KEY_DOWN: {
            lv_group_focus_next(group);
            break;
        }
        default: {
            break;
        }
    }
}

static void msgbox_cancel(lv_event_t *event) {
    lv_obj_t *mbox = lv_event_get_current_target(event);
    lv_obj_t *target = lv_event_get_target(event);
    lv_group_t *group = lv_obj_get_group(target);
    if (!group) return;
    lv_obj_t *btns = lv_msgbox_get_btns(mbox);
    if (btns && !lv_obj_has_flag(btns, LV_OBJ_FLAG_HIDDEN) &&
        !lv_btnmatrix_has_btn_ctrl(btns, 0, LV_BTNMATRIX_CTRL_DISABLED)) {
        lv_msgbox_close(mbox);
    }
}

static void msgbox_destroy(lv_event_t *event) {
    lv_group_t *group = lv_event_get_user_data(event);
    app_ui_remove_modal_group(group->user_data, group);
    lv_group_remove_all_objs(group);
    lv_group_del(group);
}
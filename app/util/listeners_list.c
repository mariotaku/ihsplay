#include "listeners_list.h"

array_list_t *listeners_list_create() {
    return array_list_create(sizeof(registered_listener_t), 16);
}

void listeners_list_destroy(array_list_t *list) {
    for (int i = 0, j = array_list_size(list); i < j; ++i) {
        registered_listener_t *l = array_list_get(list, i);
        if (l->refcounter.lock != NULL) {
            refcounter_destroy(&l->refcounter);
        }
    }
    array_list_destroy(list);
}

void listeners_list_add(array_list_t *list, const void *listener, void *context) {
    registered_listener_t *item = array_list_add(list, -1);
    memset(item, 0, sizeof(registered_listener_t));
    refcounter_init(&item->refcounter);
    item->listener = listener;
    item->context = context;
}

void listeners_list_remove(array_list_t *list, const void *listener) {
    registered_listener_t *to_unregister = NULL;
    int i;
    for (i = array_list_size(list) - 1; i >= 0; --i) {
        registered_listener_t *item = array_list_get(list, i);
        if (item->listener == listener) {
            to_unregister = item;
            break;
        }
    }
    if (to_unregister == NULL || to_unregister->refcounter.counter == 0) return;
    if (refcounter_unref(&to_unregister->refcounter)) {
        refcounter_destroy(&to_unregister->refcounter);
        array_list_remove(list, i);
    }
}
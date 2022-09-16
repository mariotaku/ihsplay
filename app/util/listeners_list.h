#pragma once

#include "array_list.h"
#include "refcounter.h"

typedef struct registered_listener_t {
    const void *listener;
    void *context;
    refcounter_t refcounter;
} registered_listener_t;

array_list_t * listeners_list_create();

void listeners_list_destroy(array_list_t *list);

void listeners_list_add(array_list_t *list, const void *listener, void *context);

void listeners_list_remove(array_list_t *list, const void *listener);

#define listeners_list_notify(lst, t, f, ...) {                                     \
    for(int i = array_list_size(lst) - 1; i >= 0; i--) {                            \
        registered_listener_t *reg = array_list_get(lst, i);                        \
        if (reg->refcounter.counter == 0) {                                         \
            continue;                                                               \
        }                                                                           \
        const t *l = (const t*) reg->listener;                                      \
        refcounter_ref(&reg->refcounter);                                           \
        if (l->f) l->f(__VA_ARGS__, reg->context);                                  \
        if (refcounter_unref(&reg->refcounter)){                                    \
            refcounter_destroy(&reg->refcounter);                                   \
            array_list_remove(lst, i);                                              \
        }                                                                           \
    }                                                                               \
}

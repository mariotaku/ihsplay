#include "array_list.h"

#include <memory.h>
#include <stdlib.h>

struct array_list_t {
    void *data;
    size_t item_size;
    int capacity;
    int size;
};

static void ensure_capacity(array_list_t *list, int new_size);

static inline void *item_at(array_list_t *list, int index);

static inline size_t items_offset(array_list_t *list, int size);

array_list_t *array_list_create(size_t item_size, int initial_capacity) {
    array_list_t *list = calloc(1, sizeof(array_list_t));
    list->data = malloc(initial_capacity * item_size);
    list->item_size = item_size;
    list->capacity = initial_capacity;
    list->size = 0;
    return list;
}

void array_list_destroy(array_list_t *list) {
    free(list->data);
    free(list);
}

void *array_list_get(array_list_t *list, int index) {
    if (index >= list->size || index < 0) return NULL;
    return item_at(list, index);
}

void *array_list_add(array_list_t *list, int insert_before) {
    if (insert_before > list->size) return NULL;
    ensure_capacity(list, list->size + 1);
    if (insert_before < 0) {
        list->size += 1;
        return item_at(list, list->size - 1);
    } else {
        memmove(item_at(list, insert_before + 1), item_at(list, insert_before), items_offset(list, list->size - insert_before));
        list->size += 1;
        return item_at(list, insert_before);
    }
}

void array_list_remove(array_list_t *list, int index) {
    if (index > list->size) return;
    if (index < list->size - 1) {
        memmove(item_at(list, index), item_at(list, index + 1), items_offset(list, list->size - index));
    }
    list->size -= 1;
}

int array_list_size(const array_list_t *list) {
    return list->size;
}

void array_list_qsort(array_list_t *list, array_list_compare_fn compare) {
    qsort(list->data, list->size, list->item_size, compare);
}

int array_list_bsearch(const array_list_t *list, const void *compare_value, array_list_compare_fn compare) {
    void *found = bsearch(compare_value, list->data, list->size, list->item_size, compare);
    if (found == NULL) {
        return -1;
    }
    return (int) ((found - list->data) / list->item_size);
}

static void ensure_capacity(array_list_t *list, int new_size) {
    if (list->capacity >= new_size) return;
    list->capacity *= 2;
    list->data = realloc(list->data, items_offset(list, list->capacity));
}

static inline void *item_at(array_list_t *list, int index) {
    return list->data + items_offset(list, index);
}

static inline size_t items_offset(array_list_t *list, int size) {
    return size * list->item_size;
}
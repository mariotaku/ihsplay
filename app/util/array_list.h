#pragma once

#include <stddef.h>

typedef struct array_list_t array_list_t;

typedef int (*array_list_compare_fn)(const void *a, const void *b);

array_list_t *array_list_create(size_t item_size, int initial_capacity);

void array_list_destroy(array_list_t *list);

void *array_list_get(array_list_t *list, int index);

void *array_list_add(array_list_t *list, int insert_before);

void array_list_remove(array_list_t *list, int index);

int array_list_size(const array_list_t *list);

void array_list_qsort(array_list_t *list, array_list_compare_fn compare);

int array_list_bsearch(const array_list_t *list, const void *compare_value, array_list_compare_fn compare);

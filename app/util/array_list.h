#pragma once

#include <stddef.h>

typedef struct array_list_t array_list_t;

array_list_t *array_list_create(size_t item_size, int initial_capacity);

void array_list_destroy(array_list_t *list);

void *array_list_get(array_list_t *list, int index);

void *array_list_add(array_list_t *list, int index);

void array_list_remove(array_list_t *list, int index);

int array_list_size(const array_list_t *list);
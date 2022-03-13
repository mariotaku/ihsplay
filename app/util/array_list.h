#pragma once

#include <stddef.h>

typedef struct array_list_t {

} array_list_t;

array_list_t *array_list_create(size_t item_size, size_t initial_capacity);

void array_list_destroy(array_list_t *list);

void *array_list_get(array_list_t *list);
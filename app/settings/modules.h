#pragma once

#include <stdbool.h>

typedef struct array_list_t array_list_t;

typedef struct str_list_t {
    int count;
    char **elements;
    char *raw;
} str_list_t;

typedef struct module_info_t {
    char *section;
    char *name;
    bool has_audio;
    bool has_video;
    int weight;
    str_list_t modules;
    str_list_t conflicts;
} module_info_t;

array_list_t *modules_load();

void modules_destroy(array_list_t *list);

bool module_conflicts(const module_info_t *a, const module_info_t *b);
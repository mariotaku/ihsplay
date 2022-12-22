#pragma once

#include <stdbool.h>

#include "util/os_info.h"
#include "util/version_info.h"

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

    version_constraint_t os_version;
} module_info_t;

array_list_t *modules_load(const os_info_t *os_info);

void modules_destroy(array_list_t *list);

bool module_conflicts(const module_info_t *a, const module_info_t *b);
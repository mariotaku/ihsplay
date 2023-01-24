#pragma once

#include "version_info.h"

typedef struct os_info_t {
    char *name;
    version_info_t version;
} os_info_t;

int os_info_get(os_info_t *info);

void os_info_clear(os_info_t *info);
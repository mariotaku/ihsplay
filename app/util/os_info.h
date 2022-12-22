#pragma once

#include "version_info.h"

typedef struct os_info_t {
    version_info_t version;
} os_info_t;

int os_info_get(os_info_t *info);
#include <stdlib.h>

#include "os_info.h"

void os_info_clear(os_info_t *info) {
    if (info->name != NULL) {
        free(info->name);
    }
}
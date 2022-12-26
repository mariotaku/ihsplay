#include <string.h>
#include "util/os_info.h"

int os_info_get(os_info_t *info) {
    memset(info, 0, sizeof(*info));
    info->version.major = -1;
    info->version.minor = -1;
    info->version.patch = -1;
    return 0;
}
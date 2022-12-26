#include "util/client_info.h"

bool client_info_load(client_info_t *info) {
    client_info_load_default(info);
    return true;
}
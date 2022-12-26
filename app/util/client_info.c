#include <stdlib.h>
#include <string.h>
#include "client_info.h"

static const uint8_t secretKey[32] = {
        11, 45, 14, 19, 19, 8, 1, 0,
        11, 45, 14, 19, 19, 8, 1, 0,
        11, 45, 14, 19, 19, 8, 1, 0,
        11, 45, 14, 19, 19, 8, 1, 0,
};

bool client_info_load_default(client_info_t *info) {
    memset(info, 0, sizeof(*info));
    info->config.deviceName = "IHSplay";
    // TODO: generate random number and store them
    info->config.deviceId = 11451419190810;
    info->config.secretKey = secretKey;
    return true;
}

void client_info_clear(client_info_t *info) {
    if (info->name != NULL) {
        free(info->name);
    }
    memset(info, 0, sizeof(*info));
}

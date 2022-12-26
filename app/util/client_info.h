#pragma once

#include "ihslib.h"

typedef struct client_info_t {
    uint64_t device_id;
    uint8_t secret_key[32];
    char *name;
    IHS_ClientConfig config;
} client_info_t;


bool client_info_load(client_info_t *info);

bool client_info_load_default(client_info_t *info);

void client_info_clear(client_info_t *info);
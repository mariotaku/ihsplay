#pragma once

#include <ihslib.h>

typedef struct app_t app_t;
typedef struct host_manager_t host_manager_t;
typedef struct array_list_t array_list_t;

typedef struct host_manager_listener_t {
    void (*hosts_reloaded)(array_list_t *list, void *context);

    void (*session_started)(const IHS_SessionInfo *config, void *context);
} host_manager_listener_t;

host_manager_t *host_manager_create(app_t *app);

void host_manager_destroy(host_manager_t *manager);

void host_manager_discovery_start(host_manager_t *manager);

void host_manager_discovery_stop(host_manager_t *manager);

void host_manager_request_session(host_manager_t *manager, const IHS_HostInfo *host);

void host_manager_register_listener(host_manager_t *manager, const host_manager_listener_t *listener, void *context);

void host_manager_unregister_listener(host_manager_t *manager, const host_manager_listener_t *listener);
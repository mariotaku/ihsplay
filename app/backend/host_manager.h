#pragma once

#include <ihslib.h>

typedef struct app_t app_t;
typedef struct host_manager_t host_manager_t;
typedef struct array_list_t array_list_t;

typedef enum host_manager_hosts_change {
    HOST_MANAGER_HOSTS_NEW,
    HOST_MANAGER_HOSTS_UPDATE
} host_manager_hosts_change;

typedef struct host_manager_listener_t {
    void (*hosts_changed)(array_list_t *list, host_manager_hosts_change change, int index, void *context);

    void (*session_started)(const IHS_HostInfo *host, const IHS_SessionInfo *config, void *context);

    void (*session_start_failed)(const IHS_HostInfo *host, IHS_StreamingResult result, void *context);

    void (*authorized)(const IHS_HostInfo *host, uint64_t steam_id, void *context);

    void (*authorization_failed)(const IHS_HostInfo *host, IHS_AuthorizationResult result, void *context);
} host_manager_listener_t;

host_manager_t *host_manager_create(app_t *app);

void host_manager_destroy(host_manager_t *manager);

void host_manager_discovery_start(host_manager_t *manager);

void host_manager_discovery_stop(host_manager_t *manager);

array_list_t *host_manager_get_hosts(host_manager_t *manager);

void host_manager_session_request(host_manager_t *manager, const IHS_HostInfo *host);

void host_manager_register_listener(host_manager_t *manager, const host_manager_listener_t *listener, void *context);

void host_manager_unregister_listener(host_manager_t *manager, const host_manager_listener_t *listener);

void host_manager_authorization_request(host_manager_t *manager, const IHS_HostInfo *host, const char *pin);

bool host_manager_authorization_cancel(host_manager_t *manager);
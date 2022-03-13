#include "app.h"
#include "hosts_manager.h"

typedef struct registered_listener_t {
    const host_manager_listener_t *listener;
    void *context;
} registered_listener_t;

struct host_manager_t {
    app_t *app;
    SDL_TimerID timer;
    lv_ll_t hosts;
    lv_ll_t listeners;
};

static Uint32 discovery_timer(Uint32 interval, void *param);

static void client_host_discovered(IHS_Client *client, IHS_HostInfo host, void *context);

static void client_host_discovered_main(app_t *app, void *data);

static const IHS_ClientDiscoveryCallbacks client_callbacks = {
        .discovered = client_host_discovered
};

host_manager_t *host_manager_create(app_t *app) {
    host_manager_t *manager = calloc(1, sizeof(host_manager_t));
    manager->app = app;
    _lv_ll_init(&manager->hosts, sizeof(IHS_HostInfo));
    _lv_ll_init(&manager->listeners, sizeof(registered_listener_t));
    return manager;
}

void host_manager_destroy(host_manager_t *manager) {
    _lv_ll_clear(&manager->listeners);
    _lv_ll_clear(&manager->hosts);
    free(manager);
}

void host_manager_discovery_start(host_manager_t *manager) {
    if (manager->timer) return;
    manager->timer = SDL_AddTimer(0, discovery_timer, manager);
    IHS_ClientSetDiscoveryCallbacks(manager->app->client, &client_callbacks, manager);
}

void host_manager_discovery_stop(host_manager_t *manager) {
    if (!manager->timer) return;
    SDL_RemoveTimer(manager->timer);
    manager->timer = 0;
    IHS_ClientSetDiscoveryCallbacks(manager->app->client, NULL, NULL);
}

void host_manager_register_listener(host_manager_t *manager, const host_manager_listener_t *listener, void *context) {
    registered_listener_t *item = _lv_ll_ins_tail(&manager->listeners);
    item->listener = listener;
    item->context = context;
}

void host_manager_unregister_listener(host_manager_t *manager, const host_manager_listener_t *listener) {
    registered_listener_t *item = NULL;
    _LV_LL_READ_BACK(&manager->listeners, item) {
        if (item->listener == listener) break;
    }
    if (item == NULL) return;
    _lv_ll_remove(&manager->listeners, item);
}

static Uint32 discovery_timer(Uint32 interval, void *param) {
    host_manager_t *manager = param;
    IHS_ClientDiscoveryBroadcast(manager->app->client);
    return 10000;
}

static void client_host_discovered(IHS_Client *client, IHS_HostInfo host, void *context) {
    host_manager_t *manager = context;
    IHS_HostInfo *host_copy = malloc(sizeof(IHS_HostInfo));
    *host_copy = host;
    app_run_on_main(manager->app, client_host_discovered_main, host_copy);
}

static void client_host_discovered_main(app_t *app, void *data) {
    IHS_HostInfo *host = data;
    IHS_HostInfo *item = NULL;
    _LV_LL_READ(&app->hosts_manager->hosts, item) {
        if (item->clientId == host->clientId) break;
    }
    if (!item) {
        item = _lv_ll_ins_tail(&app->hosts_manager->hosts);
    }
    *item = *host;
    free(host);
}
#include <assert.h>
#include "app.h"
#include "hosts_manager.h"
#include "util/array_list.h"

typedef struct registered_listener_t {
    const host_manager_listener_t *listener;
    void *context;
} registered_listener_t;

struct host_manager_t {
    app_t *app;
    IHS_Client *client;
    SDL_TimerID timer;
    array_list_t *hosts;
    lv_ll_t listeners;
};

static Uint32 discovery_timer(Uint32 interval, void *param);

static void client_host_discovered(IHS_Client *client, IHS_HostInfo host, void *context);

static void client_host_discovered_main(app_t *app, void *data);

static void client_log(IHS_LogLevel level, const char *message);

static const IHS_ClientDiscoveryCallbacks discovery_callbacks = {
        .discovered = client_host_discovered
};

host_manager_t *host_manager_create(app_t *app) {
    host_manager_t *manager = calloc(1, sizeof(host_manager_t));
    manager->app = app;
    manager->client = IHS_ClientCreate(&clientConfig);
    manager->hosts = array_list_create(sizeof(IHS_HostInfo), 16);
    _lv_ll_init(&manager->listeners, sizeof(registered_listener_t));

    IHS_ClientSetLogFunction(manager->client, client_log);
    IHS_ClientThreadedStart(manager->client);
    return manager;
}

void host_manager_destroy(host_manager_t *manager) {
    IHS_ClientDestroy(manager->client);
    _lv_ll_clear(&manager->listeners);
    array_list_destroy(manager->hosts);
    free(manager);
}

void host_manager_discovery_start(host_manager_t *manager) {
    if (manager->timer) return;
    manager->timer = SDL_AddTimer(0, discovery_timer, manager);
    IHS_ClientSetDiscoveryCallbacks(manager->client, &discovery_callbacks, manager);
}

void host_manager_discovery_stop(host_manager_t *manager) {
    if (!manager->timer) return;
    SDL_RemoveTimer(manager->timer);
    manager->timer = 0;
    IHS_ClientSetDiscoveryCallbacks(manager->client, NULL, NULL);
}

void host_manager_start_session(host_manager_t *manager, const IHS_HostInfo *host) {
    IHS_StreamingRequest request = {
            .gamepadCount = 0,
            .audioChannelCount = 2,
            .streamingEnable.audio = true,
            .streamingEnable.video = true,
            .streamingEnable.input = true,
            .maxResolution.x = 1920,
            .maxResolution.y = 1080,
    };
    IHS_ClientStreamingRequest(manager->client, host, &request);
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
    IHS_ClientDiscoveryBroadcast(manager->client);
    return 10000;
}

static void client_host_discovered(IHS_Client *client, IHS_HostInfo host, void *context) {
    host_manager_t *manager = context;
    IHS_HostInfo *host_copy = malloc(sizeof(IHS_HostInfo));
    *host_copy = host;
    app_run_on_main(manager->app, client_host_discovered_main, host_copy);
}

static void client_host_discovered_main(app_t *app, void *data) {
    host_manager_t *manager = app->hosts_manager;
    IHS_HostInfo *host = data;
    IHS_HostInfo *item = NULL;
    array_list_t *hosts = manager->hosts;
    for (int i = 0, j = array_list_size(hosts); i < j; ++i) {
        item = array_list_get(hosts, i);
        if (item->clientId == host->clientId) break;
    }
    if (item == NULL) {
        item = array_list_add(hosts, -1);
    }
    assert(item != NULL);
    *item = *host;
    free(host);

    registered_listener_t *listener = NULL;
    _LV_LL_READ(&manager->listeners, listener) {
        listener->listener->hosts_reloaded(hosts, listener->context);
    }
}

static void client_log(IHS_LogLevel level, const char *message) {
    printf("[IHSClient] %s", message);
}

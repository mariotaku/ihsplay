#include <assert.h>

#include "app.h"
#include "hosts_manager.h"

#include "util/array_list.h"
#include "util/refcounter.h"

typedef struct registered_listener_t {
    const host_manager_listener_t *listener;
    void *context;
    refcounter_t refcounter;
} registered_listener_t;

struct host_manager_t {
    app_t *app;
    IHS_Client *client;
    SDL_TimerID timer;
    array_list_t *hosts;
    lv_ll_t listeners;
    lv_ll_t listeners_unreg;
};

static Uint32 discovery_timer(Uint32 interval, void *param);

static void client_host_discovered(IHS_Client *client, IHS_HostInfo host, void *context);

static void client_streaming_success(IHS_Client *client, IHS_SocketAddress address, const uint8_t *sessionKey,
                                     size_t sessionKeyLen, void *context);

static void client_streaming_failed(IHS_Client *client, IHS_StreamingResult result, void *context);

static void client_host_discovered_main(app_t *app, void *data);

static void client_streaming_success_main(app_t *app, void *data);

static void client_log(IHS_LogLevel level, const char *message);

static void unref_listeners_cleanup(host_manager_t *manager);

static const IHS_ClientDiscoveryCallbacks discovery_callbacks = {
        .discovered = client_host_discovered
};

static const IHS_ClientStreamingCallbacks streaming_callbacks = {
        .success = client_streaming_success,
        .failed =client_streaming_failed,
};

#define LISTENERS_NOTIFY(m, f, ...) {                                               \
    registered_listener_t *l;                                                       \
    _LV_LL_READ_BACK(&((m)->listeners), l) {                                        \
        refcounter_ref(&l->refcounter);                                             \
        if (l->listener->f) l->listener->f(__VA_ARGS__, l->context);                \
        if (refcounter_unref(&l->refcounter)) {                                     \
            registered_listener_t **p = _lv_ll_ins_tail(&m->listeners_unreg);       \
            *p = l;                                                                 \
        }                                                                           \
    }                                                                               \
}

host_manager_t *host_manager_create(app_t *app) {
    host_manager_t *manager = SDL_calloc(1, sizeof(host_manager_t));
    manager->app = app;
    manager->client = IHS_ClientCreate(&app->client_config);
    manager->hosts = array_list_create(sizeof(IHS_HostInfo), 16);
    _lv_ll_init(&manager->listeners, sizeof(registered_listener_t));
    _lv_ll_init(&manager->listeners_unreg, sizeof(registered_listener_t *));

    IHS_ClientSetLogFunction(manager->client, client_log);
    IHS_ClientSetStreamingCallbacks(manager->client, &streaming_callbacks, manager);
    IHS_ClientSetDiscoveryCallbacks(manager->client, &discovery_callbacks, manager);
    IHS_ClientThreadedStart(manager->client);
    return manager;
}

void host_manager_destroy(host_manager_t *manager) {
    IHS_ClientStop(manager->client);
    IHS_ClientThreadedJoin(manager->client);
    IHS_ClientDestroy(manager->client);
    unref_listeners_cleanup(manager);
    _lv_ll_clear(&manager->listeners);
    array_list_destroy(manager->hosts);
    SDL_free(manager);
}

void host_manager_discovery_start(host_manager_t *manager) {
    if (manager->timer) return;
    manager->timer = SDL_AddTimer(0, discovery_timer, manager);
}

void host_manager_discovery_stop(host_manager_t *manager) {
    if (!manager->timer) return;
    SDL_RemoveTimer(manager->timer);
    manager->timer = 0;
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
    refcounter_init(&item->refcounter);
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
    if (refcounter_unref(&item->refcounter)) {
        refcounter_destroy(&item->refcounter);
        lv_mem_free(item);
    }
}

static Uint32 discovery_timer(Uint32 interval, void *param) {
    host_manager_t *manager = param;
    IHS_ClientDiscoveryBroadcast(manager->client);
    return 10000;
}

static void client_host_discovered(IHS_Client *client, IHS_HostInfo host, void *context) {
    LV_UNUSED(client);
    host_manager_t *manager = context;
    IHS_HostInfo *host_copy = SDL_calloc(1, sizeof(IHS_HostInfo));
    *host_copy = host;
    char *ip_str = IHS_IPAddressToString(&host.address.ip);
    fprintf(stderr, "[IHSClient] discovered %s: %s\n", ip_str, host.hostname);
    free(ip_str);
    app_run_on_main(manager->app, client_host_discovered_main, host_copy);
}

static void client_streaming_success(IHS_Client *client, IHS_SocketAddress address, const uint8_t *sessionKey,
                                     size_t sessionKeyLen, void *context) {
    LV_UNUSED(client);
    host_manager_t *manager = context;
    IHS_SessionConfig *config = SDL_calloc(1, sizeof(IHS_SessionConfig));
    config->address = address;
    SDL_memcpy(config->sessionKey, sessionKey, sessionKeyLen);
    config->sessionKeyLen = sessionKeyLen;
    app_run_on_main(manager->app, client_streaming_success_main, config);
}

static void client_streaming_failed(IHS_Client *client, IHS_StreamingResult result, void *context) {
    LV_UNUSED(client);
    fprintf(stderr, "[IHSClient] failed to start streaming: %u\n", result);
}

static void client_host_discovered_main(app_t *app, void *data) {
    host_manager_t *manager = app->hosts_manager;
    IHS_HostInfo *host = data;
    IHS_HostInfo *info = NULL;
    array_list_t *hosts = manager->hosts;
    for (int i = 0, j = array_list_size(hosts); i < j; ++i) {
        info = array_list_get(hosts, i);
        if (info->clientId == host->clientId) break;
    }
    if (info == NULL) {
        info = array_list_add(hosts, -1);
    }
    assert(info != NULL);
    *info = *host;
    SDL_free(host);

    LISTENERS_NOTIFY(manager, hosts_reloaded, hosts);

    unref_listeners_cleanup(manager);
}

static void client_streaming_success_main(app_t *app, void *data) {
    host_manager_t *manager = app->hosts_manager;
    IHS_SessionConfig *config = data;

    LISTENERS_NOTIFY(manager, session_started, config);
    SDL_free(config);

    unref_listeners_cleanup(manager);
}

static void client_log(IHS_LogLevel level, const char *message) {
    printf("[IHSClient] %s\n", message);
}

static void unref_listeners_cleanup(host_manager_t *manager) {
    registered_listener_t **item_p;
    _LV_LL_READ(&manager->listeners_unreg, item_p) {
        registered_listener_t *item = *item_p;
        refcounter_destroy(&item->refcounter);
        lv_mem_free(item);
    }
    _lv_ll_clear(&manager->listeners_unreg);
}
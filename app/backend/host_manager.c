#include <assert.h>

#include "app.h"
#include "host_manager.h"

#include "util/array_list.h"
#include "util/refcounter.h"
#include "util/listeners_list.h"

struct host_manager_t {
    app_t *app;
    IHS_Client *client;
    SDL_TimerID timer;
    array_list_t *hosts;
    array_list_t *listeners;
};

static void client_host_discovered(IHS_Client *client, IHS_HostInfo host, void *context);

static void client_streaming_success(IHS_Client *client, IHS_SocketAddress address, const uint8_t *sessionKey,
                                     size_t sessionKeyLen, void *context);

static void client_streaming_failed(IHS_Client *client, IHS_StreamingResult result, void *context);

static void client_host_discovered_main(app_t *app, void *data);

static void client_streaming_success_main(app_t *app, void *data);

static int compare_host_name(const void *a, const void *b);

static const IHS_ClientDiscoveryCallbacks discovery_callbacks = {
        .discovered = client_host_discovered
};

static const IHS_ClientStreamingCallbacks streaming_callbacks = {
        .success = client_streaming_success,
        .failed = client_streaming_failed,
};

host_manager_t *host_manager_create(app_t *app) {
    host_manager_t *manager = SDL_calloc(1, sizeof(host_manager_t));
    manager->app = app;
    manager->client = IHS_ClientCreate(&app->client_config);
    manager->hosts = array_list_create(sizeof(IHS_HostInfo), 16);
    manager->listeners = listeners_list_create();
    IHS_ClientSetLogFunction(manager->client, app_ihs_log);
    IHS_ClientSetStreamingCallbacks(manager->client, &streaming_callbacks, manager);
    IHS_ClientSetDiscoveryCallbacks(manager->client, &discovery_callbacks, manager);
    return manager;
}

void host_manager_destroy(host_manager_t *manager) {
    IHS_ClientStop(manager->client);
    IHS_ClientThreadedJoin(manager->client);
    IHS_ClientDestroy(manager->client);
    listeners_list_destroy(manager->listeners);
    array_list_destroy(manager->hosts);
    SDL_free(manager);
}

void host_manager_discovery_start(host_manager_t *manager) {
    IHS_ClientStartDiscovery(manager->client, 10000);
}

void host_manager_discovery_stop(host_manager_t *manager) {
    IHS_ClientStopDiscovery(manager->client);
}

array_list_t *host_manager_get_hosts(host_manager_t *manager) {
    return manager->hosts;
}

void host_manager_request_session(host_manager_t *manager, const IHS_HostInfo *host) {
    IHS_StreamingRequest request = {
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
    listeners_list_add(manager->listeners, listener, context);
}

void host_manager_unregister_listener(host_manager_t *manager, const host_manager_listener_t *listener) {
    listeners_list_remove(manager->listeners, listener);
}

void host_manager_add_fake(host_manager_t *manager) {
    IHS_HostInfo *host_copy = SDL_calloc(1, sizeof(IHS_HostInfo));
    snprintf(host_copy->hostname, 64, "Fake #%04u", rand() % 10000);
    host_copy->universe = IHS_SteamUniversePublic;
    host_copy->clientId = random();
    host_copy->instanceId = random();
    IHS_IPAddressFromString(&host_copy->address.ip, "114.114.114.114");
    host_copy->address.port = 23076;
    app_run_on_main(manager->app, client_host_discovered_main, host_copy);
}

static void client_host_discovered(IHS_Client *client, IHS_HostInfo host, void *context) {
    (void) client;
    host_manager_t *manager = context;
    IHS_HostInfo *host_copy = SDL_calloc(1, sizeof(IHS_HostInfo));
    *host_copy = host;
    app_run_on_main(manager->app, client_host_discovered_main, host_copy);
}

static void client_streaming_success(IHS_Client *client, IHS_SocketAddress address, const uint8_t *sessionKey,
                                     size_t sessionKeyLen, void *context) {
    (void) client;
    host_manager_t *manager = context;
    IHS_SessionInfo *config = SDL_calloc(1, sizeof(IHS_SessionInfo));
    config->address = address;
    SDL_memcpy(config->sessionKey, sessionKey, sessionKeyLen);
    config->sessionKeyLen = sessionKeyLen;
    app_run_on_main(manager->app, client_streaming_success_main, config);
}

static void client_streaming_failed(IHS_Client *client, IHS_StreamingResult result, void *context) {
    (void) client;
    (void) context;
    app_ihs_vlog(IHS_LogLevelError, "Client", "failed to start streaming: %u", result);
}

static void client_host_discovered_main(app_t *app, void *data) {
    host_manager_t *manager = app->hosts_manager;
    IHS_HostInfo *host = data;
    IHS_HostInfo *info = NULL;
    array_list_t *hosts = manager->hosts;
    int insert_index = -1, update_index = -1;
    int old_size = array_list_size(hosts);
    for (int i = 0, j = old_size; i < j; ++i) {
        IHS_HostInfo *item = array_list_get(hosts, i);
        if (item->clientId == host->clientId) {
            info = item;
            update_index = i;
            break;
        } else if (compare_host_name(item, host) > 0) {
            if (insert_index == -1) {
                insert_index = i;
            }
        }
    }
    host_manager_hosts_change change_type;
    int change_index;
    if (info == NULL) {
        app_ihs_vlog(IHS_LogLevelDebug, "Hosts", "New host discovered: %s", host->hostname);
        info = array_list_add(hosts, insert_index);
        change_type = HOST_MANAGER_HOSTS_NEW;
        change_index = insert_index < 0 ? old_size : insert_index;
    } else {
        change_type = HOST_MANAGER_HOSTS_UPDATE;
        assert(update_index >= 0);
        change_index = update_index;
    }
    assert(info != NULL);
    *info = *host;
    SDL_free(host);

    listeners_list_notify(manager->listeners, host_manager_listener_t, hosts_reloaded, hosts, change_type,
                          change_index);
}

static void client_streaming_success_main(app_t *app, void *data) {
    host_manager_t *manager = app->hosts_manager;
    IHS_SessionInfo *config = data;

    listeners_list_notify(manager->listeners, host_manager_listener_t, session_started, config);
    SDL_free(config);
}


static int compare_host_name(const void *a, const void *b) {
    const IHS_HostInfo *info1 = a;
    const IHS_HostInfo *info2 = b;
    return strncasecmp(info1->hostname, info2->hostname, 63);
}
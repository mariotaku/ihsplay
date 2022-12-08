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
    for (int i = 0, j = array_list_size(hosts); i < j; ++i) {
        IHS_HostInfo *item = array_list_get(hosts, i);
        if (item->clientId == host->clientId) {
            info = item;
            break;
        }
    }
    if (info == NULL) {
        app_ihs_vlog(IHS_LogLevelDebug, "Hosts", "New host discovered: %s", host->hostname);
        info = array_list_add(hosts, -1);
    }
    assert(info != NULL);
    *info = *host;
    SDL_free(host);

    listeners_list_notify(manager->listeners, host_manager_listener_t, hosts_reloaded, hosts);
}

static void client_streaming_success_main(app_t *app, void *data) {
    host_manager_t *manager = app->hosts_manager;
    IHS_SessionInfo *config = data;

    listeners_list_notify(manager->listeners, host_manager_listener_t, session_started, config);
    SDL_free(config);
}


#include <stdlib.h>
#include <stdio.h>

#include "app.h"
#include "ui/app_ui.h"

static void client_log(IHS_LogLevel level, const char *message);

static void client_host_discovered(IHS_Client *client, IHS_HostInfo host, void *context);

static const IHS_ClientCallbacks client_callbacks = {
        .hostDiscovered = client_host_discovered
};

app_t *app_create(lv_disp_t *disp) {
    app_t *app = calloc(1, sizeof(app_t));
    app->running = true;
    app->client = IHS_ClientCreate(&clientConfig);
    IHS_ClientSetLogFunction(app->client, client_log);
    IHS_ClientSetCallbacks(app->client, &client_callbacks, app);
    IHS_ClientThreadedStart(app->client);
    app->ui = app_ui_create(app, disp);
    return app;
}

void app_destroy(app_t *app) {
    app_ui_destroy(app->ui);
    IHS_ClientDestroy(app->client);
    free(app);
}

void app_quit(app_t *app) {
    app->running = false;
}

void app_discovery_broadcast(app_t *app) {
    IHS_ClientDiscoveryBroadcast(app->client);
}

static void client_log(IHS_LogLevel level, const char *message) {
    printf("[IHSClient] %s", message);
}

static void client_host_discovered(IHS_Client *client, IHS_HostInfo host, void *context) {

}
#include "app.h"

typedef struct bus_blocking_action_t {
    app_run_action_fn action;

    void *data;
    SDL_mutex *mutex;
    SDL_cond *cond;
    bool done;
} bus_action_sync_t;

static void invoke_action_sync(app_t *app, void *data);

void app_post_event(app_t *app, app_event_type_t type, void *data1, void *data2) {
    (void) app;
    SDL_Event event;
    event.user.type = type;
    event.user.data1 = data1;
    event.user.data2 = data2;
    SDL_PushEvent(&event);
}

void app_run_on_main(app_t *app, app_run_action_fn action, void *data) {
    (void) app;
    app_post_event(app, APP_RUN_ON_MAIN, action, data);
}

void app_run_on_main_sync(app_t *app, app_run_action_fn action, void *data) {
    bus_action_sync_t sync = {
            .action = action,
            .data = data,
            .mutex = SDL_CreateMutex(),
            .cond = SDL_CreateCond(),
            .done = false,
    };
    app_run_on_main(app, invoke_action_sync, &sync);
    SDL_LockMutex(sync.mutex);
    while (!sync.done) {
        SDL_CondWait(sync.cond, sync.mutex);
    }
    SDL_UnlockMutex(sync.mutex);
    SDL_DestroyMutex(sync.mutex);
    SDL_DestroyCond(sync.cond);
}

static void invoke_action_sync(app_t *app, void *data) {
    (void) app;
    bus_action_sync_t *sync = data;
    SDL_LockMutex(sync->mutex);
    sync->action(app, sync->data);
    sync->done = true;
    SDL_CondSignal(sync->cond);
    SDL_UnlockMutex(sync->mutex);
}
#include <assert.h>
#include "input_manager.h"
#include "app.h"

static void insert_controller(input_manager_t *manager, SDL_JoystickID id, SDL_GameController *controller);

static void remove_controller_at(input_manager_t *manager, size_t index);

static int manager_index(const input_manager_t *manager, SDL_JoystickID id);

static int instance_id_compar(const void *id, const void *item);

input_manager_t *input_manager_create() {
    input_manager_t *manager = calloc(1, sizeof(input_manager_t));
    manager->controllers_size = 0;
    manager->controllers_cap = 8;
    manager->controllers = calloc(manager->controllers_cap, sizeof(opened_controller_t));
    return manager;
}

void input_manager_destroy(input_manager_t *manager) {
    for (int i = 0; i < manager->controllers_size; i++) {
        SDL_GameControllerClose(manager->controllers[i].controller);
    }
    free(manager->controllers);
    free(manager);
}

void input_manager_sdl_gamepad_added(input_manager_t *manager, int which) {
    SDL_GameController *controller = SDL_GameControllerOpen(which);
    SDL_Joystick *joystick = SDL_GameControllerGetJoystick(controller);
    SDL_JoystickID id = SDL_JoystickInstanceID(joystick);
    insert_controller(manager, id, controller);
    app_ihs_vlog(IHS_LogLevelInfo, "Input", "Gamepad #%d: %s added.", id, SDL_JoystickName(joystick));
}

void input_manager_sdl_gamepad_removed(input_manager_t *manager, SDL_JoystickID which) {
    int index = manager_index(manager, which);
    assert(index >= 0);
    SDL_GameControllerClose(manager->controllers[index].controller);
    remove_controller_at(manager, index);
    app_ihs_vlog(IHS_LogLevelInfo, "Input", "Gamepad #%d removed.", which);
}

static void insert_controller(input_manager_t *manager, SDL_JoystickID id, SDL_GameController *controller) {
    if (manager->controllers_size + 1 > manager->controllers_cap) {
        manager->controllers_cap = manager->controllers_cap * 2;
        manager->controllers = realloc(manager->controllers, manager->controllers_cap * sizeof(opened_controller_t));
    }
    int insert_after;
    for (insert_after = (int) (manager->controllers_size - 1); insert_after >= 0; insert_after--) {
        if (id >= manager->controllers[insert_after].id) {
            break;
        }
    }
    int move_count = (int) manager->controllers_size - insert_after - 1;
    if (move_count > 0) {
        memmove(&manager->controllers[insert_after + 2], &manager->controllers[insert_after + 1],
                move_count * sizeof(opened_controller_t));
    }
    manager->controllers[insert_after + 1].id = id;
    manager->controllers[insert_after + 1].controller = controller;
    manager->controllers_size += 1;
}

static void remove_controller_at(input_manager_t *manager, size_t index) {
    if (index < manager->controllers_size - 1) {
        memmove(&manager->controllers[index], &manager->controllers[index + 1],
                (manager->controllers_size - index - 1) * sizeof(opened_controller_t));
    }
    manager->controllers_size -= 1;
}

static int manager_index(const input_manager_t *manager, SDL_JoystickID id) {
    void *offset = bsearch(&id, manager->controllers, manager->controllers_size, sizeof(opened_controller_t),
                           instance_id_compar);
    if (offset == NULL) {
        return -1;
    }
    return (int) ((offset - (void *) manager->controllers) / sizeof(opened_controller_t));
}

static int instance_id_compar(const void *id, const void *item) {
    return *((SDL_JoystickID *) id) - ((const opened_controller_t *) item)->id;
}
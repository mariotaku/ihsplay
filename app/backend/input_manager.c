#include <assert.h>
#include "input_manager.h"
#include "app.h"

#include "ihslib/hid/sdl.h"
#include "logging.h"

static void insert_controller(input_manager_t *manager, SDL_JoystickID id, SDL_GameController *controller);

static void remove_controller_at(input_manager_t *manager, size_t index);

static int manager_index(const input_manager_t *manager, SDL_JoystickID id);

static int js_count(void *context);

static int js_index(SDL_JoystickID instance_id, void *context);

static SDL_JoystickID js_instance_id(int index, void *context);

static SDL_GameController *js_controller(int index, void *context);

static int instance_id_compar(const void *id, const void *item);

static const IHS_HIDProviderSDLDeviceList hid_device_list = {
        .count = js_count,
        .index = js_index,
        .instanceId = js_instance_id,
        .controller = js_controller,
};

input_manager_t *input_manager_create() {
    input_manager_t *manager = calloc(1, sizeof(input_manager_t));
    array_list_init(&manager->controllers, sizeof(opened_controller_t), 8);
    manager->hid_provider = IHS_HIDProviderSDLCreateUnmanaged(&hid_device_list, manager);
    return manager;
}

void input_manager_destroy(input_manager_t *manager) {
    IHS_HIDProviderSDLDestroy(manager->hid_provider);
    for (int i = 0, j = array_list_size(&manager->controllers); i < j; i++) {
        opened_controller_t *controller = array_list_get(&manager->controllers, i);
        SDL_GameControllerClose(controller->controller);
    }
    array_list_deinit(&manager->controllers);
    free(manager);
}

IHS_HIDProvider *input_manager_get_hid_provider(input_manager_t *manager) {
    return manager->hid_provider;
}

void input_manager_sdl_gamepad_added(input_manager_t *manager, int which) {
    SDL_GameController *controller = SDL_GameControllerOpen(which);
    SDL_Joystick *joystick = SDL_GameControllerGetJoystick(controller);
    SDL_JoystickID id = SDL_JoystickInstanceID(joystick);
    insert_controller(manager, id, controller);
    commons_log_info("Input", "Gamepad #%d: %s added.", id, SDL_JoystickName(joystick));
}

void input_manager_sdl_gamepad_removed(input_manager_t *manager, SDL_JoystickID which) {
    int index = manager_index(manager, which);
    assert(index >= 0);
    opened_controller_t *controller = array_list_get(&manager->controllers, index);
    SDL_GameControllerClose(controller->controller);
    remove_controller_at(manager, index);
    commons_log_info("Input", "Gamepad #%d removed.", which);
}

size_t input_manager_sdl_gamepad_count(const input_manager_t *manager) {
    return array_list_size(&manager->controllers);
}

void input_manager_ignore_next_mouse_movement(input_manager_t *manager) {
    manager->ignore_next_mouse_movement = true;
}

bool input_manager_get_and_reset_mouse_movement(input_manager_t *manager) {
    bool ignore = manager->ignore_next_mouse_movement;
    manager->ignore_next_mouse_movement = false;
    return ignore;
}

static void insert_controller(input_manager_t *manager, SDL_JoystickID id, SDL_GameController *controller) {
    int insert_after;
    for (insert_after = (int) (array_list_size(&manager->controllers) - 1); insert_after >= 0; insert_after--) {
        opened_controller_t *item = array_list_get(&manager->controllers, insert_after);
        if (id >= item->id) {
            break;
        }
    }
    opened_controller_t *new_item = array_list_add(&manager->controllers, insert_after + 1);
    new_item->id = id;
    new_item->controller = controller;
}

static void remove_controller_at(input_manager_t *manager, size_t index) {
    array_list_remove(&manager->controllers, (int) index);
}

static int manager_index(const input_manager_t *manager, SDL_JoystickID id) {
    return array_list_bsearch(&manager->controllers, &id, instance_id_compar);
}

static int js_count(void *context) {
    input_manager_t *manager = context;
    return (int) array_list_size(&manager->controllers);
}

static int js_index(SDL_JoystickID instance_id, void *context) {
    input_manager_t *manager = context;
    for (int i = 0, j = array_list_size(&manager->controllers); i < j; i++) {
        opened_controller_t *item = array_list_get(&manager->controllers, i);
        if (item->id == instance_id) {
            return i;
        }
    }
    return -1;
}

static SDL_JoystickID js_instance_id(int index, void *context) {
    input_manager_t *manager = context;
    opened_controller_t *item = array_list_get(&manager->controllers, index);
    if (item == NULL) {
        return -1;
    }
    return item->id;
}

static SDL_GameController *js_controller(int index, void *context) {
    input_manager_t *manager = context;
    opened_controller_t *item = array_list_get(&manager->controllers, index);
    if (item == NULL) {
        return NULL;
    }
    return item->controller;
}

static int instance_id_compar(const void *id, const void *item) {
    return *((SDL_JoystickID *) id) - ((const opened_controller_t *) item)->id;
}
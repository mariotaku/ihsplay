#include <SDL.h>
#include <lvgl.h>

#include "app.h"
#include "lvgl/display.h"
#include "lvgl/mouse.h"

static void process_events();

static app_t *app = NULL;

int main(int argc, char *argv[]) {
    (void) argc;
    (void) argv;
    SDL_Init(SDL_INIT_VIDEO);
    SDL_RegisterEvents(APP_EVENT_SIZE);
    lv_init();

    int w = 1920, h = 1080;
    SDL_DisplayMode mode;
    SDL_GetDisplayMode(0, 0, &mode);
    /* Get display size. Fallback to 1920x1080 if failed. */
    if (mode.w > 0 && mode.h > 0) {
        w = mode.w;
        h = mode.h;
    }
    /* Caveat: Don't use SDL_WINDOW_FULLSCREEN_DESKTOP on webOS. On older platforms it's not supported. */
    SDL_Window *window = SDL_CreateWindow("myapp", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, w, h,
                                          SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_FULLSCREEN_DESKTOP);
    lv_disp_t *disp = app_lv_disp_init(window);
    lv_disp_set_default(disp);
    app_lv_mouse_init();

    app = app_create(disp);

    while (app->running) {
        process_events();
        lv_task_handler();
        SDL_Delay(1);
    }

    app_destroy(app);

    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}

static void process_events() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_QUIT:
                app_quit(app);
                break;
            case APP_RUN_ON_MAIN: {
                void (*action)(app_t *, void *) = event.user.data1;
                void *data = event.user.data2;
                action(app, data);
                break;
            }
            default:
                break;
        }
    }
}


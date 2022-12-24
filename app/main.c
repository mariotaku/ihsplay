#include <SDL.h>
#include <lvgl.h>

#include "app.h"

#include "ui/app_ui.h"

#include "lvgl/display.h"

#include "ss4s.h"

#include "backend/host_manager.h"
#include "backend/stream_manager.h"
#include "backend/input_manager.h"

#include "logging/app_logging.h"
#include "util/os_info.h"

static void process_events();

static void logging_init();

static app_t *app = NULL;

int main(int argc, char *argv[]) {
    logging_init();
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER | SDL_INIT_HAPTIC);

    os_info_t os_info;
    os_info_get(&os_info);
    if (version_info_valid(&os_info.version)) {
        app_log_info("APP", "System version: %d.%d.%d", os_info.version.major, os_info.version.minor,
                     os_info.version.patch);
    }

    app_settings_t settings;
    app_settings_init(&settings, &os_info);

    SS4S_Config ss4s_config = {
            .audioDriver = settings.audio_driver,
            .videoDriver = settings.video_driver,
            .loggingFunction = app_ss4s_logf,
    };
    SS4S_Init(argc, argv, &ss4s_config);
    IHS_Init();
    SDL_RegisterEvents((APP_EVENT_LAST - APP_EVENT_BEGIN) + 1);
    lv_init();

    int w = 1920, h = 1080;
    SDL_DisplayMode mode;
    SDL_GetDisplayMode(0, 0, &mode);
    /* Get display size. Fallback to 1920x1080 if failed. */
    if (mode.w > 0 && mode.h > 0) {
        w = mode.w;
        h = mode.h;
    }
    Uint32 fullscreen_flag;
#ifdef TARGET_WEBOS
    fullscreen_flag = SDL_WINDOW_FULLSCREEN;
#else
    bool windowed = SDL_getenv("IHSPLAY_WINDOWED") != NULL;
    if (windowed) {
        w = 1280;
        h = 720;
        fullscreen_flag = SDL_WINDOW_RESIZABLE;
    } else {
        fullscreen_flag = SDL_WINDOW_FULLSCREEN_DESKTOP;
    }
#endif
    /* Caveat: Don't use SDL_WINDOW_FULLSCREEN_DESKTOP on webOS. On older platforms it's not supported. */
    SDL_Window *window = SDL_CreateWindow("IHSplay", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, w, h,
                                          SDL_WINDOW_ALLOW_HIGHDPI | fullscreen_flag);
    SS4S_PostInit(argc, argv);

    lv_disp_t *disp = app_lv_disp_init(window);
    lv_disp_set_default(disp);

    app = app_create(&settings, disp);

    while (app->running) {
        process_events();
        lv_task_handler();
        SDL_Delay(1);
    }

    app_destroy(app);

    app_lv_disp_deinit(disp);

    app_settings_deinit(&settings);

    SDL_DestroyWindow(window);
    SDL_Quit();
    IHS_Quit();

    SS4S_Quit();
    return 0;
}

static void process_events() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_CONTROLLERDEVICEADDED:
            case SDL_CONTROLLERDEVICEREMOVED:
            case SDL_CONTROLLERBUTTONUP:
            case SDL_CONTROLLERBUTTONDOWN:
            case SDL_KEYUP:
            case SDL_KEYDOWN: {
                app_sdl_gamepad_event(app, &event);
                break;
            }
            case SDL_QUIT: {
                app_quit(app);
                break;
            }
            case APP_RUN_ON_MAIN: {
                void (*action)(app_t *, void *) = event.user.data1;
                void *data = event.user.data2;
                action(app, data);
                break;
            }
            default: {
                if (event.type > APP_UI_EVENT_BEGIN && event.type < APP_UI_EVENT_LAST) {
                    app_ui_event_data_t data = {.data1 = event.user.data1, .data2 = event.user.data2};
                    app_ui_send_event(app->ui, event.type, &data);
                }
                break;
            }
        }

        stream_manager_handle_event(app->stream_manager, &event);
    }
}

static void logging_init() {
    app_logging_init();
    lv_log_register_print_cb(app_lv_log);
}
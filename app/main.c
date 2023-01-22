#include <SDL.h>
#include <lvgl.h>

#include "app.h"
#include "config.h"

#include "ui/app_ui.h"

#include "lvgl/display.h"
#include "lvgl/ext/lv_dir_focus.h"

#include "ss4s.h"

#include "backend/host_manager.h"
#include "backend/stream_manager.h"
#include "backend/input_manager.h"

#include "logging/app_logging.h"
#include "util/os_info.h"

#if IHSPLAY_FEATURE_LIBCEC

#include "cec/cec_support.h"

#endif

static void process_events();

static void logging_init();

static app_t *app = NULL;

static bool use_windowed();

int main(int argc, char *argv[]) {
    logging_init();
    app_preinit(argc, argv);
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
    lv_dir_focus_register();

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
#elif IHSPLAY_FEATURE_FORCE_FULLSCREEN
    fullscreen_flag = SDL_WINDOW_FULLSCREEN_DESKTOP;
#else
    bool windowed = use_windowed();
    if (windowed) {
        w = 1920;
        h = 1080;
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
    app->os_info = os_info;

#if IHSPLAY_FEATURE_LIBCEC
    cec_support_ctx_t *cec = cec_support_create(app);
#endif

    while (app->running) {
        process_events();
        uint32_t next_delay = lv_task_handler();
        SDL_Delay(stream_manager_is_active(app->stream_manager) ? 1 : next_delay);
    }

#if IHSPLAY_FEATURE_LIBCEC
    cec_support_destroy(cec);
#endif

    app_destroy(app);

    app_lv_disp_deinit(disp);

    app_settings_deinit(&settings);

    SDL_DestroyWindow(window);
    IHS_Quit();

    SS4S_Quit();

    SDL_Quit();
    app_logging_deinit();
    return 0;
}

static void process_events() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_KEYUP:
            case SDL_KEYDOWN:
            case SDL_MOUSEMOTION:
            case SDL_MOUSEBUTTONDOWN:
            case SDL_MOUSEBUTTONUP:
            case SDL_CONTROLLERAXISMOTION:
            case SDL_CONTROLLERBUTTONDOWN:
            case SDL_CONTROLLERBUTTONUP:
            case SDL_CONTROLLERDEVICEADDED:
            case SDL_CONTROLLERDEVICEREMOVED: {
                bool intercept_by_stream = stream_manager_intercept_event(app->stream_manager, &event);
                if (!intercept_by_stream) {
                    app_sdl_input_event(app, &event);
                }
                stream_manager_handle_event(app->stream_manager, &event);
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
                    if (!app_ui_dispatch_event(app->ui, event.type, &data)) {
                        app_log_debug("UI", "Unhandled UI event 0x%x", event.type);
                    }
                }
                break;
            }
        }
    }
}

static void logging_init() {
    app_logging_init();
    lv_log_register_print_cb(app_lv_log);
}

static bool use_windowed() {
    const char *v = SDL_getenv("IHSPLAY_WINDOWED");
    if (v == NULL) {
        return false;
    }
    return strcmp(v, "1") == 0 || strcmp(v, "true") == 0;
}
#include <SDL.h>
#include <lvgl.h>
#include <assert.h>

#include "app.h"

#include "ui/app_ui.h"

#include "lvgl/display.h"
#include "lvgl/theme.h"

#include "ss4s.h"

#include "backend/host_manager.h"
#include "backend/stream_manager.h"

static void process_events();

static app_t *app = NULL;

int main(int argc, char *argv[]) {
    const static SS4S_Config config = {.audioDriver = "alsa", .videoDriver = "mmal"};
    SS4S_Init(argc, argv, &config);
    printf("Audio sink: %s\n", SS4S_GetAudioModuleName());
    printf("Video sink: %s\n", SS4S_GetVideoModuleName());
    IHS_Init();
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER);
    SDL_RegisterEvents((APP_EVENT_LAST - APP_EVENT_BEGIN) + 1);
    lv_log_register_print_cb(app_lv_log);
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
    fullscreen_flag = SDL_WINDOW_FULLSCREEN_DESKTOP;
#endif
    /* Caveat: Don't use SDL_WINDOW_FULLSCREEN_DESKTOP on webOS. On older platforms it's not supported. */
    SDL_Window *window = SDL_CreateWindow("myapp", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, w, h,
                                          SDL_WINDOW_ALLOW_HIGHDPI | fullscreen_flag);
    SS4S_PostInit(argc, argv);

    lv_disp_t *disp = app_lv_disp_init(window);
    lv_disp_set_default(disp);
    lv_theme_t theme;
    lv_memset_00(&theme, sizeof(lv_theme_t));
    lv_theme_set_parent(&theme, lv_disp_get_theme(disp));
    app_theme_init(&theme);
    lv_disp_set_theme(disp, &theme);

    app = app_create(disp);
    app_theme_set_ui(&theme, app->ui);

    while (app->running) {
        process_events();
        lv_task_handler();
        SDL_Delay(1);
    }

    app_destroy(app);

    app_theme_deinit(&theme);

    app_lv_disp_deinit(disp);

    SDL_DestroyWindow(window);
    SDL_Quit();
    IHS_Quit();

    SS4S_Quit();
    return 0;
}

static void process_events() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        stream_manager_handle_event(app->stream_manager, &event);
        switch (event.type) {
            case SDL_MOUSEMOTION: {
                IHS_Session *session = stream_manager_active_session(app->stream_manager);
                if (!session) {
                    break;
                }
                if (stream_manager_is_overlay_opened(app->stream_manager)) {
                    break;
                }
                if (app->settings.relmouse) {
                    IHS_SessionSendMouseMovement(session, event.motion.xrel, event.motion.yrel);
                } else {
                    int w, h;
                    SDL_GetWindowSize(app->ui->window, &w, &h);
                    IHS_SessionSendMousePosition(session, (float) event.motion.x / (float) w,
                                                 (float) event.motion.y / (float) h);
                }
                break;
            }
            case SDL_MOUSEBUTTONDOWN:
            case SDL_MOUSEBUTTONUP: {
                IHS_Session *session = stream_manager_active_session(app->stream_manager);
                if (!session) {
                    break;
                }
                if (stream_manager_is_overlay_opened(app->stream_manager)) {
                    break;
                }
                IHS_StreamInputMouseButton button = 0;
                switch (event.button.button) {
                    case SDL_BUTTON_LEFT:
                        button = IHS_MOUSE_BUTTON_LEFT;
                        break;
                    case SDL_BUTTON_RIGHT:
                        button = IHS_MOUSE_BUTTON_RIGHT;
                        break;
                    case SDL_BUTTON_MIDDLE:
                        button = IHS_MOUSE_BUTTON_MIDDLE;
                        break;
                    case SDL_BUTTON_X1:
                        button = IHS_MOUSE_BUTTON_X1;
                        break;
                    case SDL_BUTTON_X2:
                        button = IHS_MOUSE_BUTTON_X2;
                        break;
                }
                if (button != 0) {
                    if (event.button.state == SDL_RELEASED) {
                        IHS_SessionSendMouseUp(session, button);
                    } else {
                        IHS_SessionSendMouseDown(session, button);
                    }
                }
                break;
            }
            case SDL_MOUSEWHEEL: {
                IHS_Session *session = stream_manager_active_session(app->stream_manager);
                if (!session) {
                    break;
                }
                if (stream_manager_is_overlay_opened(app->stream_manager)) {
                    break;
                }
                Sint32 x = event.wheel.x, y = event.wheel.y;
                if (event.wheel.direction == SDL_MOUSEWHEEL_FLIPPED) {
                    x *= -1;
                    y *= -1;
                }
                if (x != 0) {
                    IHS_SessionSendMouseWheel(session, x < 0 ? IHS_MOUSE_WHEEL_LEFT : IHS_MOUSE_WHEEL_RIGHT);
                }
                if (y != 0) {
                    IHS_SessionSendMouseWheel(session, y > 0 ? IHS_MOUSE_WHEEL_UP : IHS_MOUSE_WHEEL_DOWN);
                }
                break;
            }
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
    }
}


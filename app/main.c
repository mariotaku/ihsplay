#include <SDL.h>
#include <lvgl.h>
#include <src/draw/sdl/lv_draw_sdl_utils.h>
#include "app.h"

static void process_events();

static lv_disp_t *display_init(SDL_Window *window);

static void flush_cb(lv_disp_drv_t *disp_drv, const lv_area_t *area, lv_color_t *src);

static app_t *app = NULL;

int main(int argc, char *argv[]) {
    (void) argc;
    (void) argv;
    SDL_Init(SDL_INIT_VIDEO);
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
    lv_disp_t *disp = display_init(window);
    lv_disp_set_default(disp);

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
            default:
                break;
        }
    }
}

static lv_disp_t *display_init(SDL_Window *window) {
    int width, height;
    SDL_GetWindowSize(window, &width, &height);
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1");
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    lv_disp_draw_buf_t *draw_buf = malloc(sizeof(lv_disp_draw_buf_t));
    SDL_Texture *texture = lv_draw_sdl_create_screen_texture(renderer, width, height);
    lv_disp_draw_buf_init(draw_buf, texture, NULL, width * height);
    lv_disp_drv_t *driver = malloc(sizeof(lv_disp_drv_t));
    lv_disp_drv_init(driver);

    lv_draw_sdl_drv_param_t *param = lv_mem_alloc(sizeof(lv_draw_sdl_drv_param_t));
    param->renderer = renderer;
    driver->user_data = param;
    driver->draw_buf = draw_buf;
    driver->flush_cb = flush_cb;
    driver->hor_res = width;
    driver->ver_res = height;
    SDL_SetRenderTarget(renderer, texture);
    lv_disp_t *disp = lv_disp_drv_register(driver);
    return disp;
}

static void flush_cb(lv_disp_drv_t *disp_drv, const lv_area_t *area, lv_color_t *src) {
    LV_UNUSED(src);
    if (area->x2 < 0 || area->y2 < 0 ||
        area->x1 > disp_drv->hor_res - 1 || area->y1 > disp_drv->ver_res - 1) {
        lv_disp_flush_ready(disp_drv);
        return;
    }

    if (lv_disp_flush_is_last(disp_drv)) {
        lv_draw_sdl_drv_param_t *param = disp_drv->user_data;
        SDL_Renderer *renderer = param->renderer;
        SDL_Texture *texture = disp_drv->draw_buf->buf1;
        SDL_SetRenderTarget(renderer, NULL);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
        SDL_RenderClear(renderer);
        SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
        SDL_RenderCopy(renderer, texture, NULL, NULL);
        SDL_RenderPresent(renderer);
        SDL_SetRenderTarget(renderer, texture);
    }
    lv_disp_flush_ready(disp_drv);
}
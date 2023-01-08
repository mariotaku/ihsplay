#include "display.h"

#include <src/draw/sdl/lv_draw_sdl.h>

static void flush_cb(lv_disp_drv_t *disp_drv, const lv_area_t *area, lv_color_t *src);

lv_disp_t *app_lv_disp_init(SDL_Window *window) {
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
    param->user_data = window;
    driver->user_data = param;
    driver->draw_buf = draw_buf;
    driver->flush_cb = flush_cb;
    driver->hor_res = width;
    driver->ver_res = height;
    SDL_SetRenderTarget(renderer, texture);
    lv_disp_t *disp = lv_disp_drv_register(driver);
    disp->bg_color = lv_color_black();
    disp->bg_opa = LV_OPA_TRANSP;
    return disp;
}

void app_lv_disp_deinit(lv_disp_t *disp) {
    lv_disp_drv_t *drv = disp->driver;

    lv_draw_ctx_t *draw_ctx = drv->draw_ctx;
    drv->draw_ctx_deinit(drv, draw_ctx);
    free(draw_ctx);

    lv_disp_draw_buf_t *draw_buf = drv->draw_buf;
    SDL_DestroyTexture(draw_buf->buf1);
    free(draw_buf);

    lv_draw_sdl_drv_param_t *param = drv->user_data;
    SDL_DestroyRenderer(param->renderer);
    free(param);

    // Set act_scr to NULL to suppress warning when we remove the display
    disp->act_scr = NULL;

    lv_disp_remove(disp);

    free(drv);
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
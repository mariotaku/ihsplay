#include "app_ui.h"
#include "app_ui_font.h"

static lv_font_t *load_font(const lv_ft_info_t *info, lv_coord_t size);

void app_ui_fontset_set_default_size(const app_ui_t *ui, app_ui_fontset_t *set) {
    set->size.small = LV_DPX(14);
    set->size.normal = LV_DPX(16);
    set->size.large = LV_DPX(21);
    set->size.xlarge = LV_DPX(32);
    set->size.huge = LV_DPX(96);
}


void app_ui_fontset_init_mem(app_ui_fontset_t *set, const char *name, const void *mem, size_t size) {
    lv_ft_info_t ft_info = {.name = name, .mem = mem, .mem_size = size, .style = FT_FONT_STYLE_NORMAL,
            .weight = set->size.normal};
    if (lv_ft_font_init(&ft_info)) {
        set->normal = ft_info.font;
    }
    set->xlarge = load_font(&ft_info, set->size.xlarge);
    set->large = load_font(&ft_info, set->size.large);
    set->huge = load_font(&ft_info, set->size.huge);
    set->small = load_font(&ft_info, set->size.small);
}

void app_ui_fontset_deinit(app_ui_fontset_t *set) {
    lv_ft_font_destroy(set->small);
    lv_ft_font_destroy(set->normal);
    lv_ft_font_destroy(set->large);
    lv_ft_font_destroy(set->xlarge);
    lv_ft_font_destroy(set->huge);
}

static lv_font_t *load_font(const lv_ft_info_t *info, lv_coord_t size) {
    lv_ft_info_t ft_info = *info;
    ft_info.font = NULL;
    ft_info.weight = size;
    if (!lv_ft_font_init(&ft_info)) {
        LV_LOG_ERROR("Failed to load font");
    }
    return ft_info.font;
}
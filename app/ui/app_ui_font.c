#include "app_ui.h"

void app_ui_fontset_set_default_size(const app_ui_t *ui, app_ui_fontset_t *set) {
    set->size.small = 18;
    set->size.normal = 32;
    set->size.large = 36;
}

void app_ui_fontset_init_mem(app_ui_fontset_t *set, const char *name, const void *mem, size_t size) {
    lv_ft_info_t ft_info = {.name = name, .mem = mem, .mem_size = size, .style = FT_FONT_STYLE_NORMAL,
            .weight = set->size.normal};
    if (lv_ft_font_init(&ft_info)) {
        set->normal = ft_info.font;
    }
    lv_ft_info_t ft_info_lg = {.name = name, .mem = mem, .mem_size = size, .style = FT_FONT_STYLE_NORMAL,
            .weight = set->size.large};
    if (lv_ft_font_init(&ft_info_lg)) {
        set->large = ft_info_lg.font;
    }
    lv_ft_info_t ft_info_sm = {.name = name, .mem = mem, .mem_size = size, .style = FT_FONT_STYLE_NORMAL,
            .weight = set->size.small};
    if (lv_ft_font_init(&ft_info_sm)) {
        set->small = ft_info_sm.font;
    }
    LV_ASSERT_NULL(set->small);
    LV_ASSERT_NULL(set->normal);
    LV_ASSERT_NULL(set->large);
}

void app_ui_fontset_deinit(app_ui_fontset_t *set) {
    lv_ft_font_destroy(set->small);
    lv_ft_font_destroy(set->normal);
    lv_ft_font_destroy(set->large);
}

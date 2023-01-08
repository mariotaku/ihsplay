#include "app_ui.h"
#include "app_ui_font.h"

#include <fontconfig/fontconfig.h>
#include <assert.h>

static lv_font_t *load_font(const lv_ft_info_t *info, lv_coord_t size);

static bool fontset_load_fc(app_ui_fontset_t *set, FcPattern *font);

void app_ui_fontset_set_default_size(const app_ui_t *ui, app_ui_fontset_t *set) {
    set->sizes.heading1 = LV_DPX(42);
    set->sizes.heading2 = LV_DPX(28);
    set->sizes.heading3 = LV_DPX(21);
    set->sizes.body = LV_DPX(16);
    set->sizes.small = LV_DPX(14);
    set->sizes.huge = LV_DPX(96);
}


void app_ui_fontset_init_mem(app_ui_fontset_t *set, const char *name, const void *mem, size_t size) {
    lv_ft_info_t ft_info = {.name = name, .mem = mem, .mem_size = size, .style = FT_FONT_STYLE_NORMAL};
    set->body = load_font(&ft_info, set->sizes.body);
    set->heading1 = load_font(&ft_info, set->sizes.heading1);
    set->heading2 = load_font(&ft_info, set->sizes.heading2);
    set->heading3 = load_font(&ft_info, set->sizes.heading3);
    set->small = load_font(&ft_info, set->sizes.small);
    set->huge = load_font(&ft_info, set->sizes.huge);
}

void app_ui_fontset_init_fc(app_ui_fontset_t *set, const char *name) {
    //does not necessarily have to be a specific name.  You could put anything here and Fontconfig WILL find a font for you
    FcPattern *pattern = FcNameParse((const FcChar8 *) name);
    assert(pattern != NULL);

    FcConfigSubstitute(NULL, pattern, FcMatchPattern);
    FcDefaultSubstitute(pattern);

    FcResult result;

    FcPattern *font = FcFontMatch(NULL, pattern, &result);

    fontset_load_fc(set, font);

    if (font) {
        FcPatternDestroy(font);
        font = NULL;
    }
    FcPatternDestroy(pattern);
    pattern = NULL;
#ifdef FONT_FAMILY_FALLBACK
    const i18n_entry_t *loc_entry = i18n_entry(i18n_locale());
    pattern = FcNameParse((const FcChar8 *) ((loc_entry && loc_entry->font) ? loc_entry->font : FONT_FAMILY_FALLBACK));
    if (pattern) {
        FcLangSet *ls = FcLangSetCreate();
        if (loc_entry) {
            FcLangSetAdd(ls, (const FcChar8 *) loc_entry->locale);
            FcPatternAddLangSet(pattern, FC_LANG, ls);
        }

        FcConfigSubstitute(NULL, pattern, FcMatchPattern);
        FcDefaultSubstitute(pattern);

        font = FcFontMatch(NULL, pattern, &result);
        if (font) {
            fontset.fallback = calloc(1, sizeof(app_fontset_t));
            if (fontset_load_fc(fontset.fallback, font)) {
                fontset.normal->fallback = fontset.fallback->normal;
                fontset.large->fallback = fontset.fallback->large;
                fontset.small->fallback = fontset.fallback->small;
            } else {
                free(fontset.fallback);
                fontset.fallback = NULL;
            }
            FcPatternDestroy(font);
            font = NULL;
        }
        FcLangSetDestroy(ls);
        FcPatternDestroy(pattern);
        pattern = NULL;
    }
#endif
}

void app_ui_fontset_apply_fallback(app_ui_fontset_t *set, const app_ui_fontset_t *fallback) {
    set->small->fallback = fallback->small;
    set->body->fallback = fallback->body;
    set->heading3->fallback = fallback->heading3;
    set->heading2->fallback = fallback->heading2;
    set->heading1->fallback = fallback->heading1;
    set->huge->fallback = fallback->huge;
}

void app_ui_fontset_deinit(app_ui_fontset_t *set) {
    lv_ft_font_destroy(set->small);
    lv_ft_font_destroy(set->body);
    lv_ft_font_destroy(set->heading3);
    lv_ft_font_destroy(set->heading2);
    lv_ft_font_destroy(set->heading1);
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


static bool fontset_load_fc(app_ui_fontset_t *set, FcPattern *font) {
    //The pointer stored in 'file' is tied to 'font'; therefore, when 'font' is freed, this pointer is freed automatically.
    //If you want to return the filename of the selected font, pass a buffer and copy the file name into that buffer
    FcChar8 *file = NULL;

    if (FcPatternGetString(font, FC_FILE, 0, &file) == FcResultMatch) {
        lv_ft_info_t ft_info = {.name = (char *) file, .style = FT_FONT_STYLE_NORMAL};
        set->body = load_font(&ft_info, set->sizes.body);
        set->heading1 = load_font(&ft_info, set->sizes.heading1);
        set->heading2 = load_font(&ft_info, set->sizes.heading2);
        set->heading3 = load_font(&ft_info, set->sizes.heading3);
        set->small = load_font(&ft_info, set->sizes.small);
        set->huge = load_font(&ft_info, set->sizes.huge);
        return true;
    }
    return false;
}
#include <string.h>
#include "app_settings.h"
#include "modules.h"
#include "util/array_list.h"

void app_settings_init(app_settings_t *settings) {
    memset(settings, 0, sizeof(app_settings_t));
    settings->modules = modules_load();
    settings->relmouse = true;

    // TODO: check if lib available, and handle conflicts
    const module_info_t *first_video_module = NULL, *first_audio_module = NULL;
    for (int i = 0, j = array_list_size(settings->modules); i < j; ++i) {
        const module_info_t *info = array_list_get(settings->modules, i);
        if (info->has_video && first_video_module == NULL) {
            first_video_module = info;
            if (info->has_audio) {
                break;
            }
        }
        if (info->has_audio && first_audio_module == NULL) {
            if (first_video_module != NULL && module_conflicts(first_video_module, info)) {
                continue;
            }
            first_audio_module = info;
        }
    }
    if (first_audio_module == NULL) {
        first_audio_module = first_video_module;
    }
    if (first_video_module != NULL) {
        settings->video_driver = first_video_module->modules.elements[0];
    }
    if (first_audio_module != NULL) {
        settings->audio_driver = first_audio_module->modules.elements[0];
    }
}

void app_settings_deinit(app_settings_t *settings) {
    modules_destroy(settings->modules);
}

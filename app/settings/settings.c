#include <string.h>

#include "app_settings.h"

#include "ss4s_modules.h"

#include "array_list.h"
#include "os_info.h"

void app_settings_init(app_settings_t *settings, const os_info_t *os_info) {
    memset(settings, 0, sizeof(app_settings_t));
    settings->modules = modules_load(os_info);
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
        settings->video_driver = module_first_available(first_video_module, SS4S_MODULE_CHECK_VIDEO);
    }
    if (first_audio_module != NULL) {
        settings->audio_driver = module_first_available(first_audio_module, SS4S_MODULE_CHECK_AUDIO);
    }
}

void app_settings_deinit(app_settings_t *settings) {
    modules_destroy(settings->modules);
}

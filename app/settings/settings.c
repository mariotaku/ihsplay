#include <string.h>

#include "app_settings.h"

#include "ss4s_modules.h"

#include "array_list.h"
#include "os_info.h"

void app_settings_init(app_settings_t *settings, const os_info_t *os_info) {
    memset(settings, 0, sizeof(app_settings_t));
    settings->modules = modules_load(os_info);
    settings->relmouse = true;

    const module_info_t *selected_video_module = NULL, *selected_audio_module = NULL;
    const char *selected_video_driver = NULL, *selected_audio_driver = NULL;
    for (int i = 0, j = array_list_size(settings->modules); i < j; ++i) {
        const module_info_t *info = array_list_get(settings->modules, i);
        if (info->has_video && selected_video_driver == NULL) {
            if (selected_audio_module != NULL && module_conflicts(selected_audio_module, info)) {
                continue;
            }
            selected_video_driver = module_first_available(info, SS4S_MODULE_CHECK_VIDEO);
            if (selected_video_driver != NULL) {
                selected_video_module = info;
            }
        }
        if (info->has_audio && selected_audio_driver == NULL) {
            if (selected_video_module != NULL && module_conflicts(selected_video_module, info)) {
                continue;
            }
            selected_audio_driver = module_first_available(info, SS4S_MODULE_CHECK_AUDIO);
            if (selected_audio_driver != NULL) {
                selected_audio_module = info;
            }
        }
    }
    settings->video_driver = selected_video_driver;
    settings->audio_driver = selected_audio_driver;
}

void app_settings_deinit(app_settings_t *settings) {
    modules_destroy(settings->modules);
}

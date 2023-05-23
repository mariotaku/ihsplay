#include <string.h>

#include "app_settings.h"

#include "ss4s_modules.h"

#include "array_list.h"
#include "os_info.h"

void app_settings_init(app_settings_t *settings, const os_info_t *os_info) {
    memset(settings, 0, sizeof(app_settings_t));
    // TODO: check result
    modules_load(&settings->modules, os_info);
    settings->relmouse = true;

    module_preferences_t preferences = {.audio_module = NULL, .video_module = NULL};
    module_selection_t selection = {.audio_module = NULL, .video_module = NULL};
    // TODO: check result
    module_select(&settings->modules, &preferences, &selection);
    settings->video_driver = module_info_get_id(selection.video_module);
    settings->audio_driver = module_info_get_id(selection.audio_module);
}

void app_settings_deinit(app_settings_t *settings) {
    modules_clear(&settings->modules);
}

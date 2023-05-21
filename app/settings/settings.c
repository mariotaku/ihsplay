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

    module_selection_t selection = {.audio_driver = NULL, .video_driver = NULL};
    // TODO: check result
    module_select(&settings->modules, NULL, &selection);
    settings->video_driver = selection.video_driver;
    settings->audio_driver = selection.audio_driver;
}

void app_settings_deinit(app_settings_t *settings) {
    modules_clear(&settings->modules);
}

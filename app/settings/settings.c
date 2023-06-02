#include <string.h>

#include "app_settings.h"

#include "ss4s_modules.h"

#include "array_list.h"
#include "os_info.h"

void app_settings_init(app_settings_t *settings, const os_info_t *os_info) {
    memset(settings, 0, sizeof(app_settings_t));
    // TODO: check result
    SS4S_ModulesList(&settings->modules, os_info);
    settings->enable_input = true;
    settings->relmouse = true;

    SS4S_ModulePreferences preferences = {.audio_module = NULL, .video_module = NULL};
    SS4S_ModuleSelection selection = {.audio_module = NULL, .video_module = NULL};
    // TODO: check result
    SS4S_ModulesSelect(&settings->modules, &preferences, &selection, true);
    settings->video_driver = SS4S_ModuleInfoGetId(selection.video_module);
    settings->audio_driver = SS4S_ModuleInfoGetId(selection.audio_module);
}

void app_settings_deinit(app_settings_t *settings) {
    SS4S_ModulesListClear(&settings->modules);
}

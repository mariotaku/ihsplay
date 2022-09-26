#pragma once

#include "ihslib.h"

typedef struct ihsplay_module_t {
    const IHS_StreamAudioCallbacks *(*audio)();

    const IHS_StreamVideoCallbacks *(*video)();
} ihsplay_module_t;

void module_init(int argc, char *argv[]);

void module_post_init(int argc, char *argv[]);

const IHS_StreamAudioCallbacks *module_audio_callbacks();

const IHS_StreamVideoCallbacks *module_video_callbacks();
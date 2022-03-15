#pragma once

#include <ihslib.h>

void module_init(int argc, char *argv[]);

void module_post_init(int argc, char *argv[]);

const IHS_StreamAudioCallbacks *module_audio_callbacks();

const IHS_StreamVideoCallbacks *module_video_callbacks();
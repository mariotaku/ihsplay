#pragma once

#include "ihslib.h"

typedef struct stream_media_session_t stream_media_session_t;

stream_media_session_t *stream_media_create();

void stream_media_destroy(stream_media_session_t *media);

const IHS_StreamAudioCallbacks *stream_media_audio_callbacks();

const IHS_StreamVideoCallbacks *stream_media_video_callbacks();
#include "module.h"

#include <stdlib.h>
#include <NDL_directmedia_v2.h>
#include <stdio.h>

static int audio_start(IHS_Session *session, const IHS_StreamAudioConfig *config, void *context);

static int audio_submit(IHS_Session *session, const uint8_t *data, size_t dataLen, void *context);

static void audio_stop(IHS_Session *session, void *context);

static int video_start(IHS_Session *session, const IHS_StreamVideoConfig *config, void *context);

static int video_submit(IHS_Session *session, const uint8_t *data, size_t dataLen, uint16_t sequence,
                        IHS_StreamVideoFrameFlag flags, void *context);

static void video_stop(IHS_Session *session, void *context);

static void media_unload();

static int media_reload();

static bool media_loaded = false;
static NDL_DIRECTMEDIA_DATA_INFO media_info = {
        .audio.type = 0,
        .video.type = 0
};

static const IHS_StreamAudioCallbacks audio_callbacks = {
        .start = audio_start,
        .submit = audio_submit,
        .stop = audio_stop,
};

static const IHS_StreamVideoCallbacks video_callbacks = {
        .start = video_start,
        .submit = video_submit,
        .stop = video_stop,
};

void module_init(int argc, char *argv[]) {
    NDL_DirectMediaInit(getenv("APPID"), NULL);
}

void module_post_init(int argc, char *argv[]) {

}

const IHS_StreamAudioCallbacks *module_audio_callbacks() {
    return &audio_callbacks;
}

const IHS_StreamVideoCallbacks *module_video_callbacks() {
    return &video_callbacks;
}

static int audio_start(IHS_Session *session, const IHS_StreamAudioConfig *config, void *context) {
    switch (config->codec) {
        case IHS_StreamAudioCodecMP3:
            media_info.audio.type = NDL_AUDIO_TYPE_MP3;
            break;
        case IHS_StreamAudioCodecOpus:
            media_info.audio.type = NDL_AUDIO_TYPE_OPUS;
            media_info.audio.opus.channels = (int) config->channels;
            media_info.audio.opus.sampleRate = (double) config->frequency / 1000.0f;
            break;
        default: {
            return -1;
        }
    }
    return media_reload();
}

static int audio_submit(IHS_Session *session, const uint8_t *data, size_t dataLen, void *context) {
    return NDL_DirectAudioPlay(data, dataLen, 0);
}

static void audio_stop(IHS_Session *session, void *context) {
    media_unload();
}

static int video_start(IHS_Session *session, const IHS_StreamVideoConfig *config, void *context) {
    switch (config->codec) {
        case IHS_StreamVideoCodecH264:
            media_info.video.type = NDL_VIDEO_TYPE_H264;
            break;
        case IHS_StreamVideoCodecHEVC:
            media_info.video.type = NDL_VIDEO_TYPE_H265;
            break;
        case IHS_StreamVideoCodecVP9:
            media_info.video.type = NDL_VIDEO_TYPE_VP9;
            break;
        default: {
            return -1;
        }
    }
    media_info.video.width = (int) config->width;
    media_info.video.height = (int) config->height;
    return media_reload();
}

static int video_submit(IHS_Session *session, const uint8_t *data, size_t dataLen, uint16_t sequence,
                        IHS_StreamVideoFrameFlag flags, void *context) {
    return NDL_DirectVideoPlay(data, dataLen, 0);
}

static void video_stop(IHS_Session *session, void *context) {
    media_unload();
}

static void media_load_callback(int type, long long numValue, const char *strValue) {
    printf("MediaLoadCallback type=%d, numValue=%x, strValue=%p\n", type, numValue, strValue);
}

static void media_unload() {
    if (!media_loaded) return;
    NDL_DirectMediaUnload();
    media_loaded = false;
}

static int media_reload() {
    media_unload();
    int ret = NDL_DirectMediaLoad(&media_info, media_load_callback);
    media_loaded = ret == 0;
    return ret;
}
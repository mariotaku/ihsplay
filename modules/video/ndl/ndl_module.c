#include "module.h"

#include <stdlib.h>
#include <NDL_directmedia.h>

static int audio_start(IHS_Session *session, const IHS_StreamAudioConfig *config, void *context);

static int audio_submit(IHS_Session *session, const uint8_t *data, size_t dataLen, void *context);

static void audio_stop(IHS_Session *session, void *context);

static int video_start(IHS_Session *session, const IHS_StreamVideoConfig *config, void *context);

static int video_submit(IHS_Session *session, const uint8_t *data, size_t dataLen, uint16_t sequence,
                        IHS_StreamVideoFrameFlag flags, void *context);

static void video_stop(IHS_Session *session, void *context);

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
    NDL_DIRECTAUDIO_DATA_INFO info = {
            .numChannel = config->channels,
            .bitPerSample = 16,
            .nodelay = 1,
            .upperThreshold = 48,
            .lowerThreshold = 16,
            .channel = NDL_DIRECTAUDIO_CH_MAIN,
            .srcType = NDL_DIRECTAUDIO_SRC_TYPE_AAC,
            .samplingFreq = NDL_DIRECTAUDIO_SAMPLING_FREQ_OF(config->frequency),
    };
    return NDL_DirectAudioOpen(&info);
}

static int audio_submit(IHS_Session *session, const uint8_t *data, size_t dataLen, void *context) {
    return NDL_DirectAudioPlay(data, dataLen, 0);
}

static void audio_stop(IHS_Session *session, void *context) {
    NDL_DirectAudioClose();
}

static int video_start(IHS_Session *session, const IHS_StreamVideoConfig *config, void *context) {
    NDL_DIRECTVIDEO_DATA_INFO info = {
            .width = (int) config->width,
            .height = (int) config->height,
    };
    return NDL_DirectVideoOpen(&info);
}

static int video_submit(IHS_Session *session, const uint8_t *data, size_t dataLen, uint16_t sequence,
                        IHS_StreamVideoFrameFlag flags, void *context) {
    return NDL_DirectVideoPlay(data, dataLen, 0);
}

static void video_stop(IHS_Session *session, void *context) {
    NDL_DirectVideoClose();
}
#include <stdlib.h>
#include "stream_media.h"

#include "ss4s.h"

#include <opus_multistream.h>

struct stream_media_session_t {
    SS4S_Player *player;
    OpusMSDecoder *opus_decoder;
    size_t pcm_unit_size;
    int16_t *pcm_buffer;
    int pcm_buffer_size;
};

static int audio_start(IHS_Session *session, const IHS_StreamAudioConfig *config, void *context);

static void audio_stop(IHS_Session *session, void *context);

static int audio_submit(IHS_Session *session, IHS_Buffer *data, void *context);

static int video_start(IHS_Session *session, const IHS_StreamVideoConfig *config, void *context);

static void video_stop(IHS_Session *session, void *context);

static int video_submit(IHS_Session *session, IHS_Buffer *data, IHS_StreamVideoFrameFlag flags, void *context);

static const IHS_StreamAudioCallbacks audio_callbacks = {
        .start = audio_start,
        .stop = audio_stop,
        .submit = audio_submit,
};

static const IHS_StreamVideoCallbacks video_callbacks = {
        .start = video_start,
        .stop = video_stop,
        .submit = video_submit,
};

stream_media_session_t *stream_media_create() {
    stream_media_session_t *media_session = calloc(1, sizeof(stream_media_session_t));
    media_session->player = SS4S_PlayerOpen();
    return media_session;
}

void stream_media_destroy(stream_media_session_t *media_session) {
    SS4S_PlayerClose(media_session->player);
    free(media_session);
}

const IHS_StreamAudioCallbacks *stream_media_audio_callbacks() {
    return &audio_callbacks;
}

const IHS_StreamVideoCallbacks *stream_media_video_callbacks() {
    return &video_callbacks;
}

static int audio_start(IHS_Session *session, const IHS_StreamAudioConfig *config, void *context) {
    (void) session;
    if (config->codec != IHS_StreamAudioCodecOpus) {
        return -1;
    }
    stream_media_session_t *media_session = (stream_media_session_t *) context;
    int rc;
    unsigned char mapping[2] = {0, 1};
    const int samples_per_frame = 240;
    media_session->pcm_buffer_size = samples_per_frame * 64;
    media_session->pcm_unit_size = config->channels * sizeof(int16_t);

    media_session->opus_decoder = opus_multistream_decoder_create(config->frequency, config->channels,
                                                                  1, 1, mapping, &rc);
    media_session->pcm_buffer = calloc(media_session->pcm_unit_size, media_session->pcm_buffer_size);
    SS4S_AudioInfo info = {
            .codec = SS4S_AUDIO_PCM_S16LE,
            .numOfChannels = (int) config->channels,
            .sampleRate = (int) config->frequency,
            .samplesPerFrame = samples_per_frame,
    };
    return SS4S_PlayerAudioOpen(media_session->player, &info);
}

static void audio_stop(IHS_Session *session, void *context) {
    (void) session;
    stream_media_session_t *media_session = (stream_media_session_t *) context;
    SS4S_PlayerAudioClose(media_session->player);
    opus_multistream_decoder_destroy(media_session->opus_decoder);
    free(media_session->pcm_buffer);
}

static int audio_submit(IHS_Session *session, IHS_Buffer *data, void *context) {
    (void) session;
    stream_media_session_t *media_session = (stream_media_session_t *) context;
    int decode_len = opus_multistream_decode(media_session->opus_decoder, data->data + data->offset, data->size,
                                             media_session->pcm_buffer, media_session->pcm_buffer_size, 0);
    return SS4S_PlayerAudioFeed(media_session->player, (const unsigned char *) media_session->pcm_buffer,
                                media_session->pcm_unit_size * decode_len);
}

static int video_start(IHS_Session *session, const IHS_StreamVideoConfig *config, void *context) {
    (void) session;
    stream_media_session_t *media_session = (stream_media_session_t *) context;
    SS4S_VideoCodec codec;
    switch (config->codec) {
        case IHS_StreamVideoCodecH264:
            codec = SS4S_VIDEO_H264;
            break;
        default:
            return -1;
    }
    SS4S_VideoInfo info = {
            .codec = codec,
            .width = (int) config->width,
            .height = (int) config->height,
    };
    return SS4S_PlayerVideoOpen(media_session->player, &info);
}

static void video_stop(IHS_Session *session, void *context) {
    (void) session;
    stream_media_session_t *media_session = (stream_media_session_t *) context;
    SS4S_PlayerVideoClose(media_session->player);
}

static int video_submit(IHS_Session *session, IHS_Buffer *data, IHS_StreamVideoFrameFlag flags, void *context) {
    (void) session;
    stream_media_session_t *media_session = (stream_media_session_t *) context;
    SS4S_VideoFeedFlags sflgs = 0;
    if (flags & IHS_StreamVideoFrameKeyFrame) {
        sflgs = SS4S_VIDEO_FEED_DATA_KEYFRAME;
    }
    return SS4S_PlayerVideoFeed(media_session->player, data->data + data->offset, data->size, sflgs);
}
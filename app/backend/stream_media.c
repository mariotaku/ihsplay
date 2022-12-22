#include <stdlib.h>
#include "stream_media.h"

#include "ss4s.h"
#include "util/video/sps_parser.h"
#include "stream_manager.h"
#include "app.h"

#include <opus_multistream.h>
#include <SDL2/SDL.h>

struct stream_media_session_t {
    stream_manager_t *manager;
    SDL_mutex *lock;
    SS4S_Player *player;

    SS4S_VideoInfo video_info;
    OpusMSDecoder *opus_decoder;
    size_t pcm_unit_size;
    int16_t *pcm_buffer;

    int pcm_buffer_size;
    int viewport_width, viewport_height;
    int overlay_height;
};

static int audio_start(IHS_Session *session, const IHS_StreamAudioConfig *config, void *context);

static void audio_stop(IHS_Session *session, void *context);

static int audio_submit(IHS_Session *session, IHS_Buffer *data, void *context);

static int video_start(IHS_Session *session, const IHS_StreamVideoConfig *config, void *context);

static void video_stop(IHS_Session *session, void *context);

static int video_submit(IHS_Session *session, IHS_Buffer *data, IHS_StreamVideoFrameFlag flags, void *context);

static int video_set_capture_size(IHS_Session *session, int width, int height, void *context);

static const IHS_StreamAudioCallbacks audio_callbacks = {
        .start = audio_start,
        .stop = audio_stop,
        .submit = audio_submit,
};

static const IHS_StreamVideoCallbacks video_callbacks = {
        .start = video_start,
        .stop = video_stop,
        .submit = video_submit,
        .setCaptureSize = video_set_capture_size,
};

stream_media_session_t *stream_media_create(stream_manager_t *manager) {
    stream_media_session_t *media_session = calloc(1, sizeof(stream_media_session_t));
    media_session->manager = manager;
    media_session->lock = SDL_CreateMutex();
    media_session->player = SS4S_PlayerOpen();
    return media_session;
}

void stream_media_destroy(stream_media_session_t *media_session) {
    SS4S_PlayerClose(media_session->player);
    SDL_DestroyMutex(media_session->lock);
    free(media_session);
}

void stream_media_set_viewport_size(stream_media_session_t *media_session, int width, int height) {
    SDL_LockMutex(media_session->lock);
    media_session->viewport_width = width;
    media_session->viewport_height = height;
    SDL_UnlockMutex(media_session->lock);
}

void stream_media_set_overlay_height(stream_media_session_t *media_session, int height) {
    SDL_LockMutex(media_session->lock);
    media_session->overlay_height = height;
    SDL_UnlockMutex(media_session->lock);
}

void stream_media_set_overlay_shown(stream_media_session_t *media_session, bool overlay) {
    SS4S_VideoCapabilities cap = SS4S_GetVideoCapabilities();
    if (cap & SS4S_VIDEO_CAP_TRANSFORM_UI_COMPOSITING) {
        return;
    }
    if (overlay) {
        SDL_LockMutex(media_session->lock);
        int ui_width = media_session->viewport_width, ui_height = media_session->viewport_height,
                overlay_height = media_session->overlay_height;
        if (ui_width <= 0 || ui_height <= 0 || overlay_height <= 0) {
            SDL_UnlockMutex(media_session->lock);
            return;
        }
        SS4S_VideoRect src = {
                .x = 0, .y = 0,
                .width = media_session->video_info.width,
                .height = media_session->video_info.height * (ui_height - overlay_height) / ui_height
        };
        SDL_UnlockMutex(media_session->lock);

        SS4S_VideoRect dest = {0, 0, ui_width, ui_height - overlay_height};
        SS4S_PlayerVideoSetDisplayArea(media_session->player, &src, &dest);
    } else {
        SS4S_PlayerVideoSetDisplayArea(media_session->player, NULL, NULL);
    }
}

const IHS_StreamAudioCallbacks *stream_media_audio_callbacks() {
    return &audio_callbacks;
}

const IHS_StreamVideoCallbacks *stream_media_video_callbacks() {
    return &video_callbacks;
}

static int audio_start(IHS_Session *session, const IHS_StreamAudioConfig *config, void *context) {
    (void) session;
    app_ihs_logf(IHS_LogLevelInfo, "Media", "Audio start. codec=%u, channels=%u, sampleRate=%u", config->codec,
                 config->channels, config->frequency);
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
    app_ihs_logf(IHS_LogLevelInfo, "Media", "Video start. codec=%u, width=%u, height=%u", config->codec, config->width,
                 config->height);
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
    media_session->video_info = info;
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
        sps_dimension_t dimension = {0, 0};
        bool dimension_parsed = false;
        switch (media_session->video_info.codec) {
            case SS4S_VIDEO_H264: {
                dimension_parsed = sps_parse_dimension_h264(IHS_BufferPointerAt(data, 4), &dimension);
                break;
            }
            case SS4S_VIDEO_H265: {
                dimension_parsed = sps_parse_dimension_hevc(IHS_BufferPointerAt(data, 4), &dimension);
                break;
            }
        }
        if (dimension_parsed && dimension.width != media_session->video_info.width ||
            dimension.height != media_session->video_info.height) {
            media_session->video_info.width = dimension.width;
            media_session->video_info.height = dimension.height;
            SS4S_PlayerVideoSizeChanged(media_session->player, dimension.width, dimension.height);
        }
    }
    return SS4S_PlayerVideoFeed(media_session->player, data->data + data->offset, data->size, sflgs);
}

static int video_set_capture_size(IHS_Session *session, int width, int height, void *context) {
    (void) session;
    stream_media_session_t *media_session = (stream_media_session_t *) context;
    stream_manager_set_capture_size(media_session->manager, width, height);
    return 0;
}
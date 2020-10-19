#pragma once

#define STR_(X) #X
#define STR(X) STR_(X)

#define WINDOW_WIDTH  1280
#define WINDOW_HEIGHT 720

#define FFT_SIZE      8192
#define FFT_SMOOTHING 0.6

#define GEN_VIDEO_AA     8
#define GEN_VIDEO_WIDTH  3840
#define GEN_VIDEO_HEIGHT 2160

#define OUT_VIDEO_CODEC        "libx264"
#define OUT_VIDEO_PRESET       "slow"
#define OUT_VIDEO_WIDTH        3840
#define OUT_VIDEO_HEIGHT       2160
#define OUT_VIDEO_CRF          "18"
#define OUT_VIDEO_PIXEL_FORMAT AV_PIX_FMT_YUV420P
#define OUT_VIDEO_FRAMERATE    60

#define OUT_AUDIO_CODEC         "flac"
#define OUT_AUDIO_SAMPLE_RATE   44100
#define OUT_AUDIO_CHANNELS      2
#define OUT_AUDIO_SAMPLE_FORMAT AV_SAMPLE_FMT_S16
#define OUT_AUDIO_SAMPLE_TYPE   uint16_t
#define OUT_AUDIO_OFFSET        (FFT_SIZE * 0.75)

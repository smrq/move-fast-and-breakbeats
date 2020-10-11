#pragma once

#define STR_(X) #X
#define STR(X) STR_(X)

// #define WINDOW_WIDTH           320
// #define WINDOW_HEIGHT          180

#define WINDOW_WIDTH           1280
#define WINDOW_HEIGHT          720

#define GEN_VIDEO_AA           8
#define GEN_VIDEO_WIDTH        3840
#define GEN_VIDEO_HEIGHT       2160

#define OUT_VIDEO_CODEC        "libx264"
#define OUT_VIDEO_PRESET       "fast"
#define OUT_VIDEO_WIDTH        3840
#define OUT_VIDEO_HEIGHT       2160
#define OUT_VIDEO_BITRATE      16000000
#define OUT_VIDEO_PIXEL_FORMAT AV_PIX_FMT_YUV420P
#define OUT_VIDEO_FRAMERATE    60

#define OUT_AUDIO_CODEC        "aac"
#define OUT_AUDIO_SAMPLE_RATE  48000
#define OUT_AUDIO_CHANNELS     2
#define OUT_AUDIO_BITRATE      196000

#define FFT_SIZE               8192
#define FFT_SMOOTHING          0.6

#define IN_AUDIO_OFFSET        (OUT_AUDIO_SAMPLE_RATE * 0.5 + (FFT_SIZE * 3 / 4))

#define VIS_OSC_COUNT          1024
#define VIS_GRID_SIZE          64
#define VIS_GRID_COUNT         (VIS_GRID_SIZE*VIS_GRID_SIZE)
#define VIS_MIN_FREQ           40
#define VIS_MAX_FREQ           16000

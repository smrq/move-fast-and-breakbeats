#pragma once
#include "deps/libav.h"
#include "constants.h"

enum StreamType {
	StreamType_Video,
	StreamType_Audio
};

struct Stream {
	AVStream *stream;
	AVCodec *codec;
	AVCodecContext *codecCtx;
	AVFrame *frame;

	int64_t nextPts;

	Stream(AVFormatContext *formatCtx, enum StreamType streamType, const char *codecName);
	~Stream();
};

struct VideoStream : public Stream {
	SwsContext *sws;

	VideoStream(AVFormatContext *formatCtx, const char *codecName);
	~VideoStream();
};

struct AudioStream : public Stream {
	AudioStream(AVFormatContext *formatCtx, const char *codecName);
	~AudioStream();
};

struct Encoder {
	AVFormatContext *formatCtx;
	VideoStream *videoStream;
	AudioStream *audioStream;

	Encoder(const char *filename);
	enum StreamType nextFrameType();
	void writeVideoFrame(uint8_t *pixels);
	void writeAudioFrame();
	int writeFrame(AVCodecContext *codecCtx, AVStream *stream, AVFrame *frame);
	void logPacket(const AVPacket *packet);
	~Encoder();
};

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

	int64_t startPts;
	int64_t nextPts;

	Stream(AVFormatContext *formatCtx, enum StreamType streamType, const char *codecName, int64_t startPts);
	~Stream();
};

struct VideoStream : public Stream {
	SwsContext *sws;

	VideoStream(AVFormatContext *formatCtx, const char *codecName, int64_t startPts);
	~VideoStream();
};

struct AudioStream : public Stream {
	SwrContext *swr;
	
	AudioStream(AVFormatContext *formatCtx, const char *codecName, int64_t startPts);
	~AudioStream();
};

struct Encoder {
	AVFormatContext *formatCtx;
	VideoStream *videoStream;
	AudioStream *audioStream;

	Encoder(const char *filename, int audioOffset);
	enum StreamType nextFrameType();
	bool writeVideoFrame(uint8_t *pixels);
	bool writeAudioFrame(double *signal, size_t signalSamples);
	int writeFrame(AVCodecContext *codecCtx, AVStream *stream, AVFrame *frame);
	void logPacket(const AVPacket *packet);
	~Encoder();
};

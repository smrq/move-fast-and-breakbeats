#pragma once
#include "deps/libav.h"
#include "constants.h"
#include "buffer.h"

struct Decoder {
	AVFormatContext *formatCtx;
	int streamIndex;
	AVStream *stream;
	AVCodec *codec;
	AVCodecContext *codecCtx;
	SwrContext *swr;

	Decoder(const char *filename);
	void decode(Buffer<double> *buf);
	void logPacket(const AVPacket *packet);
	~Decoder();
};

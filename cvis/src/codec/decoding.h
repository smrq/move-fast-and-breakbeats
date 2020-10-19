#pragma once
#include <vector>
#include "deps/libav.h"
#include "constants.h"

struct Decoder {
	AVFormatContext *formatCtx;
	int streamIndex;
	AVStream *stream;
	AVCodec *codec;
	AVCodecContext *codecCtx;
	SwrContext *swr;

	Decoder(const char *filename);
	void decode(std::vector<double> &buffer);
	~Decoder();
};

#include "encoding.h"

Stream::Stream(AVFormatContext *formatCtx, enum StreamType streamType, const char *codecName, int64_t startPts)
: startPts(startPts), nextPts(startPts) {
	int result;

	stream = avformat_new_stream(formatCtx, NULL);
	if (!stream) { fprintf(stderr, "Could not allocate stream\n"); exit(1); }

	codec = avcodec_find_encoder_by_name(codecName);
	if (!codec) { fprintf(stderr, "Could not find encoder for '%s'\n", codecName); exit(1); }

	codecCtx = avcodec_alloc_context3(codec);
	if (!codecCtx) { fprintf(stderr, "Could not allocate codec context\n"); exit(1); }

	AVDictionary *opts = NULL;

	switch (streamType) {
		case StreamType_Video:
			codecCtx->width = OUT_VIDEO_WIDTH;
			codecCtx->height = OUT_VIDEO_HEIGHT;
			// codecCtx->bit_rate = OUT_VIDEO_BITRATE;
			codecCtx->time_base = (AVRational){ 1, OUT_VIDEO_FRAMERATE };
			codecCtx->gop_size = OUT_VIDEO_FRAMERATE / 2;
			codecCtx->pix_fmt = OUT_VIDEO_PIXEL_FORMAT;
			codecCtx->max_b_frames = 2;
			av_dict_set(&opts, "preset", OUT_VIDEO_PRESET, 0);
			av_dict_set(&opts, "crf", OUT_VIDEO_CRF, 0);
			break;

		case StreamType_Audio:
			codecCtx->channels = OUT_AUDIO_CHANNELS;
			codecCtx->channel_layout = av_get_default_channel_layout(OUT_AUDIO_CHANNELS);
			codecCtx->sample_rate = OUT_AUDIO_SAMPLE_RATE;
			codecCtx->sample_fmt = OUT_AUDIO_SAMPLE_FORMAT;
			codecCtx->time_base = (AVRational){ 1, OUT_AUDIO_SAMPLE_RATE };
			codecCtx->strict_std_compliance = FF_COMPLIANCE_EXPERIMENTAL;
			break;
	}

	stream->time_base = codecCtx->time_base;

	result = avcodec_open2(codecCtx, codec, &opts);
	if (result < 0) { fprintf(stderr, "Could not open the codec\n"); exit(1); }

	result = avcodec_parameters_from_context(stream->codecpar, codecCtx);
	if (result < 0) { fprintf(stderr, "Could not copy the stream parameters\n"); exit(1); }

	frame = av_frame_alloc();
	if (!frame) { fprintf(stderr, "Could not allocate frame\n"); exit(1); }
}

VideoStream::VideoStream(AVFormatContext *formatCtx, const char *codecName, int64_t startPts)
: Stream(formatCtx, StreamType_Video, codecName, startPts) {
	int result;

	sws = sws_getContext(
		GEN_VIDEO_WIDTH, GEN_VIDEO_HEIGHT, AV_PIX_FMT_RGB32,
		OUT_VIDEO_WIDTH, OUT_VIDEO_HEIGHT, OUT_VIDEO_PIXEL_FORMAT,
		SWS_BICUBIC, 0, 0, 0);
	if (!sws) { fprintf(stderr, "Could not allocate scaling context\n"); exit(1); }

	frame->format = codecCtx->pix_fmt;
	frame->width = codecCtx->width;
	frame->height = codecCtx->height;

	result = av_frame_get_buffer(frame, 0);
	if (result < 0) { fprintf(stderr, "Could not allocate the video frame data\n"); exit(1); }
}

AudioStream::AudioStream(AVFormatContext *formatCtx, const char *codecName, int64_t startPts)
: Stream(formatCtx, StreamType_Audio, codecName, startPts) {
	int result;

	swr = swr_alloc();
	if (!swr) { fprintf(stderr, "Could not allocate resampler context\n"); exit(1); }
	av_opt_set_int(swr, "in_channel_count", 2, 0);
	av_opt_set_int(swr, "out_channel_count", codecCtx->channels, 0);
	av_opt_set_int(swr, "in_channel_layout", AV_CH_LAYOUT_STEREO, 0);
	av_opt_set_int(swr, "out_channel_layout", codecCtx->channel_layout, 0);
	av_opt_set_int(swr, "in_sample_rate", OUT_AUDIO_SAMPLE_RATE, 0);
	av_opt_set_int(swr, "out_sample_rate", codecCtx->sample_rate, 0);
	av_opt_set_int(swr, "in_sample_fmt", AV_SAMPLE_FMT_DBL, 0);
	av_opt_set_int(swr, "out_sample_fmt", codecCtx->sample_fmt, 0);
	swr_init(swr);
	if (!swr_is_initialized(swr)) { fprintf(stderr, "Could not initialize resampler\n"); exit(1); }	

	if (swr_get_out_samples(swr, codecCtx->frame_size) != codecCtx->frame_size) {
		fprintf(stderr, "Expected same number of input and output samples from resampler\n"); exit(1);
	}

	frame->format = codecCtx->sample_fmt;
	frame->nb_samples = codecCtx->frame_size;
	frame->channel_layout = codecCtx->channel_layout;

	result = av_frame_get_buffer(frame, 0);
	if (result < 0) { fprintf(stderr, "Could not allocate the audio frame data: %s\n", av_err2str(result)); exit(1); }
}

Stream::~Stream() {
	av_frame_free(&frame);
	avcodec_free_context(&codecCtx);
}

VideoStream::~VideoStream() {
	sws_freeContext(sws);
}

AudioStream::~AudioStream() {
	swr_free(&swr);
}

Encoder::Encoder(const char *filename, int audioOffset) {
	int result;

	avformat_alloc_output_context2(&formatCtx, NULL, NULL, filename);	
	if (!formatCtx) {
		fprintf(stderr, "Could not deduce output format from file extension: using MPEG.\n");
		avformat_alloc_output_context2(&formatCtx, NULL, "mpeg", filename);
	}
	if (!formatCtx) {
		fprintf(stderr, "Could not create format context.\n");
		exit(1);
	}

	formatCtx->strict_std_compliance = FF_COMPLIANCE_EXPERIMENTAL;

	videoStream = new VideoStream(formatCtx, OUT_VIDEO_CODEC, 0);
	audioStream = new AudioStream(formatCtx, OUT_AUDIO_CODEC, audioOffset);

	av_dump_format(formatCtx, 0, filename, 1);

	result = avio_open(&formatCtx->pb, filename, AVIO_FLAG_WRITE);
	if (result < 0) { fprintf(stderr, "Could not open '%s': %s\n", filename, av_err2str(result)); exit(1); }

	AVDictionary* opts = NULL;
	av_dict_set(&opts, "movflags", "faststart", 0);

	result = avformat_write_header(formatCtx, &opts);
	if (result < 0) { fprintf(stderr, "Error occurred when opening output file: %s\n", av_err2str(result)); exit(1); }
}

enum StreamType Encoder::nextFrameType() {
	int result = av_compare_ts(
		videoStream->nextPts, videoStream->codecCtx->time_base,
		audioStream->nextPts, audioStream->codecCtx->time_base);
	return (result <= 0) ? StreamType_Video : StreamType_Audio;
}

bool Encoder::writeVideoFrame(uint8_t *pixels) {
	int result;
	
	result = av_frame_make_writable(videoStream->frame);
	if (result < 0) { fprintf(stderr, "Could not make video frame data writable\n"); exit(1); }

	uint8_t *lastLine = pixels + (GEN_VIDEO_HEIGHT - 1) * GEN_VIDEO_WIDTH * 4;
	int stride = -GEN_VIDEO_WIDTH * 4;
	sws_scale(videoStream->sws, &lastLine, &stride, 0, GEN_VIDEO_HEIGHT, videoStream->frame->data, videoStream->frame->linesize);

	videoStream->frame->pts = videoStream->nextPts;
	videoStream->nextPts += 1;

	writeFrame(videoStream->codecCtx, videoStream->stream, videoStream->frame);

	return true;
}

bool Encoder::writeAudioFrame(double *signal, size_t signalSamples) {
	int result;
	
	result = av_frame_make_writable(audioStream->frame);
	if (result < 0) { fprintf(stderr, "Could not make audio frame data writable\n"); exit(1); }

	size_t firstSample = audioStream->nextPts - audioStream->startPts;
	size_t lastSample = firstSample + audioStream->frame->nb_samples;
	if (lastSample > signalSamples) {
		lastSample = signalSamples;
	}
	double *inData = signal + 2*firstSample;

	int convertedSamples = swr_convert(audioStream->swr,
		(uint8_t **)&audioStream->frame->data,
		audioStream->frame->nb_samples,
		(const uint8_t **)&inData,
		lastSample - firstSample);

	audioStream->frame->nb_samples = convertedSamples;
	audioStream->frame->pts = audioStream->nextPts;
	audioStream->nextPts += convertedSamples;

	writeFrame(audioStream->codecCtx, audioStream->stream, audioStream->frame);

	return lastSample < signalSamples;
}

int Encoder::writeFrame(AVCodecContext *codecCtx, AVStream *stream, AVFrame *frame) {
	int result;

	AVPacket *packet = av_packet_alloc();
	if (!packet) { fprintf(stderr, "Error allocating output packet\n"); exit(1); }

	result = avcodec_send_frame(codecCtx, frame);
	if (result < 0) { fprintf(stderr, "Error sending a frame to the encoder: %s\n", av_err2str(result)); exit(1); }

	while (result >= 0) {
		result = avcodec_receive_packet(codecCtx, packet);
		if (result == AVERROR(EAGAIN) || result == AVERROR_EOF) {
			break;
		} else if (result < 0) {
			fprintf(stderr, "Error receiving packet from encoder: %s\n", av_err2str(result));
			exit(1);
		}

		packet->stream_index = stream->index;
		// video only: (needed?)
		// packet->duration = stream->time_base.den / stream->time_base.num / stream->avg_frame_rate.num * stream->avg_frame_rate.den;

		av_packet_rescale_ts(packet, codecCtx->time_base, stream->time_base);

		logPacket(packet);

		result = av_interleaved_write_frame(formatCtx, packet);
		if (result < 0) { fprintf(stderr, "Error while writing output packet: %s\n", av_err2str(result)); exit(1); }

		av_packet_unref(packet);
	}

	av_packet_free(&packet);
	
	return result == AVERROR_EOF;
}

void Encoder::logPacket(const AVPacket *packet) {
	AVRational *time_base = &formatCtx->streams[packet->stream_index]->time_base;
	printf("pts:%s pts_time:%s dts:%s dts_time:%s duration:%s duration_time:%s stream_index:%d\n",
		av_ts2str(packet->pts), av_ts2timestr(packet->pts, time_base),
		av_ts2str(packet->dts), av_ts2timestr(packet->dts, time_base),
		av_ts2str(packet->duration), av_ts2timestr(packet->duration, time_base),
		packet->stream_index);
}

Encoder::~Encoder() {
	av_write_trailer(formatCtx);

	delete videoStream;
	// delete audioStream;
	avio_closep(&formatCtx->pb);
	avformat_free_context(formatCtx);
}

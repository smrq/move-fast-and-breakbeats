#include "decoding.h"

Decoder::Decoder(const char *filename) {
	int result;

	formatCtx = avformat_alloc_context();

	result = avformat_open_input(&formatCtx, filename, NULL, NULL);
	if (result < 0) { fprintf(stderr, "Could not open source file %s\n", filename); exit(1); }

	result = avformat_find_stream_info(formatCtx, NULL);
	if (result < 0) { fprintf(stderr, "Could not find stream info\n"); exit(1); }

	av_dump_format(formatCtx, 0, filename, 0);

	streamIndex = av_find_best_stream(formatCtx, AVMEDIA_TYPE_AUDIO, -1, 01, NULL, 0);
	if (streamIndex < 0) { fprintf(stderr, "Could not find audio stream\n"); exit(1); }

	stream = formatCtx->streams[streamIndex];
	codec = avcodec_find_decoder(stream->codecpar->codec_id);

	codecCtx = avcodec_alloc_context3(codec);
	if (!codecCtx) { fprintf(stderr, "Could not allocate codec context\n"); exit(1); }

	result = avcodec_parameters_to_context(codecCtx, stream->codecpar);
	if (result < 0) { fprintf(stderr, "Could not copy codec parameters\n"); exit(1); }

	result = avcodec_open2(codecCtx, codec, NULL);
	if (result < 0) { fprintf(stderr, "Could not open codec\n"); exit(1); }

	swr = swr_alloc();
	if (!swr) { fprintf(stderr, "Could not allocate resampler context\n"); exit(1); }
	av_opt_set_int(swr, "in_channel_count", codecCtx->channels, 0);
	av_opt_set_int(swr, "out_channel_count", 2, 0);
	av_opt_set_int(swr, "in_channel_layout", codecCtx->channel_layout, 0);
	av_opt_set_int(swr, "out_channel_layout", AV_CH_LAYOUT_STEREO, 0);
	av_opt_set_int(swr, "in_sample_rate", codecCtx->sample_rate, 0);
	av_opt_set_int(swr, "out_sample_rate", OUT_AUDIO_SAMPLE_RATE, 0);
	av_opt_set_int(swr, "in_sample_fmt", codecCtx->sample_fmt, 0);
	av_opt_set_int(swr, "out_sample_fmt", AV_SAMPLE_FMT_DBL, 0);
	swr_init(swr);
	if (!swr_is_initialized(swr)) { fprintf(stderr, "Could not initialize resampler\n"); exit(1); }	
}

void Decoder::logPacket(const AVPacket *packet) {
	AVRational *time_base = &formatCtx->streams[packet->stream_index]->time_base;
	printf("pts:%s pts_time:%s dts:%s dts_time:%s duration:%s duration_time:%s stream_index:%d\n",
		av_ts2str(packet->pts), av_ts2timestr(packet->pts, time_base),
		av_ts2str(packet->dts), av_ts2timestr(packet->dts, time_base),
		av_ts2str(packet->duration), av_ts2timestr(packet->duration, time_base),
		packet->stream_index);
}

void Decoder::decode(Buffer<double> *buf) {
	int result;

	AVPacket *packet = av_packet_alloc();
	if (!packet) { fprintf(stderr, "Could not allocate packet\n"); exit(1); }

	AVFrame *frame = av_frame_alloc();
	if (!frame) { fprintf(stderr, "Could not allocate frame\n"); exit(1); }

	while (av_read_frame(formatCtx, packet) >= 0) {
		if (packet->stream_index != streamIndex) {
			continue;
		}

		// logPacket(packet);

		result = avcodec_send_packet(codecCtx, packet);	
		if (result < 0) { fprintf(stderr, "Error sending packet to decoder: %s\n", av_err2str(result)); exit(1); }

		while (result >= 0) {
			result = avcodec_receive_frame(codecCtx, frame);
			if (result == AVERROR(EAGAIN) || result == AVERROR_EOF) {
				break;
			} else if (result < 0) {
				fprintf(stderr, "Error receiving frame from decoder: %s", av_err2str(result));
				exit(1);
			}

			int maxSamples = swr_get_out_samples(swr, frame->nb_samples);

			double *data = buf->request(2 * maxSamples);
			int samples = swr_convert(swr, (uint8_t **)&data, maxSamples, (const uint8_t**)frame->data, frame->nb_samples);
			if (result < 0) { fprintf(stderr, "Error resampling audio\n"); exit(1); }
			buf->markUsed(2 * samples);

			av_frame_unref(frame);
		}

		av_packet_unref(packet);
	}

	av_frame_free(&frame);
	av_packet_free(&packet);
}

Decoder::~Decoder() {
	swr_free(&swr);
	avcodec_close(codecCtx);
	avformat_free_context(formatCtx);
}
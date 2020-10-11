#include "constants.h"
#include "encoding.h"
#include "decoding.h"
#include "buffer.h"
#include "analyze.h"
#include "vis.h"

int main_vis();

void decode(const char *inputFilename, Buffer<double> *buffer) {
	Decoder *decoder = new Decoder(inputFilename);
	decoder->decode(buffer);
	delete decoder;
}

int main(int argc, char **argv)
{
	if (argc < 3) {
		printf("usage: %s input_audio output_video\n", argv[0]);
		exit(1);
	}

	const char *inputFilename = argv[1];
	const char *outputFilename = argv[2];

	Buffer<double> *signal = new Buffer<double>(2 * 1024 * 1024);
	decode(inputFilename, signal);

	Analyzer *analyzer = new Analyzer(FFT_SIZE, FFT_SMOOTHING);
	Encoder *encoder = new Encoder(outputFilename);
	Vis *vis = new Vis();

	uint8_t *pixels = (uint8_t *)malloc(GEN_VIDEO_WIDTH * GEN_VIDEO_HEIGHT * 4 * sizeof(uint8_t));

	for (int frame = 0; ; ++frame) {
		int sample = frame * OUT_AUDIO_SAMPLE_RATE / OUT_VIDEO_FRAMERATE - IN_AUDIO_OFFSET;
		printf("Frame %d: sample %d\n", frame, sample);
		if (sample >= (int)(signal->length / 2)) {
			break;
		}

		analyzer->analyze(signal->data, signal->length / 2, sample);
		vis->drawFrame(
			(float)frame / OUT_VIDEO_FRAMERATE,
			analyzer->timeResult, FFT_SIZE,
			analyzer->freqResult, FFT_SIZE / 2);
		vis->readPixels(pixels);
		encoder->writeVideoFrame(pixels);
	}

	delete vis;
	delete encoder;
	delete analyzer;
	delete signal;
}

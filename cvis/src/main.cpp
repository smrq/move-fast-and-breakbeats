#include <unistd.h>
#include <memory>
#include "deps/gl.h"
#include "constants.h"
#include "encoding.h"
#include "decoding.h"
#include "buffer.h"
#include "analyze.h"
#include "vis.h"

void usage(const char *x) {
	printf("usage: %s -i INPUT_AUDIO -o OUTPUT_VIDEO -f END_FRAME_IMAGE -e END_FRAME_TIME [-s INITIAL_SILENCE]\n", x);
}

int main(int argc, char **argv) {
	int c;
	char *inputFilename;
	char *outputFilename;
	char *endFrameFilename;
	double silence = 0.0;
	double endFrameTime = -1.0;

	while ((c = getopt(argc, argv, "i:o:f:e:s:")) != -1) {
		switch (c) {
			case 'i':
				inputFilename = optarg;
				break;
			case 'o':
				outputFilename = optarg;
				break;
			case 'f':
				endFrameFilename = optarg;
				break;
			case 'e':
				endFrameTime = atof(optarg);
				break;
			case 's':
				silence = atof(optarg);
				break;
			case '?':
			default:
				usage(argv[0]);
				exit(1);
		}
	}
	if (inputFilename == NULL || outputFilename == NULL || endFrameFilename == NULL || endFrameTime < 0.0) {
		usage(argv[0]);
		exit(1);
	}

	int audioOffset = VIS_AUDIO_OFFSET + (silence * OUT_AUDIO_SAMPLE_RATE);
	int endFrame = (int)(OUT_VIDEO_FRAMERATE * (endFrameTime + ((double)audioOffset / OUT_AUDIO_SAMPLE_RATE)));

	auto signal = std::make_unique<Buffer<double>>(2 * 1024 * 1024);
	auto decoder = std::make_unique<Decoder>(inputFilename);
	decoder->decode(signal.get());

	auto analyzer = std::make_unique<Analyzer>(FFT_SIZE, FFT_SMOOTHING);
	auto encoder = std::make_unique<Encoder>(outputFilename, audioOffset);

	auto window = std::make_shared<gl::Window>(WINDOW_WIDTH, WINDOW_HEIGHT, "OpenGL");
	auto vis = std::make_unique<Vis>(window, endFrameFilename);

	uint8_t *pixels = (uint8_t *)malloc(GEN_VIDEO_WIDTH * GEN_VIDEO_HEIGHT * 4 * sizeof(uint8_t));

	bool audioFinished = false;
	bool videoFinished = false;
	while (!audioFinished && !videoFinished) {
		if (!videoFinished && encoder->nextFrameType() == StreamType_Video) {
			int frame = encoder->videoStream->nextPts;
			if (frame < endFrame) {
				int sample = frame * OUT_AUDIO_SAMPLE_RATE / OUT_VIDEO_FRAMERATE - audioOffset;
				analyzer->analyze(signal->data, signal->length / 2, sample);
				vis->drawFrame(
					(float)frame / OUT_VIDEO_FRAMERATE,
					analyzer->timeResult, FFT_SIZE,
					analyzer->freqResult, FFT_SIZE / 2);
			} else {
				vis->drawEndFrame();
				videoFinished = true;
			}
			vis->readPixels(pixels);
			encoder->writeVideoFrame(pixels);
		} else {
			audioFinished = !encoder->writeAudioFrame(signal->data, signal->length / 2);
			if (encoder->audioStream->nextPts >= OUT_AUDIO_SAMPLE_RATE * 60) {
				audioFinished = true;
			}
		}
	}
}

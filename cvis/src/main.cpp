#include <memory>
#include "analyze.h"
#include "codec/decoding.h"
#include "codec/encoding.h"
#include "constants.h"
#include "deps/gl.h"
#include "glcontext.h"
#include "scenes/endframe.h"
#include "scenes/tetrik.h"

void usage(const char *x) {
	printf("usage: %s --endframe IMAGE TIME [--silence TIME] INPUT_AUDIO OUTPUT_VIDEO\n", x);
}

int main(int argc, char **argv) {
	int c;
	char *inputFilename;
	char *outputFilename;
	char *endFrameFilename;
	double silence = 0.0;
	double endFrameTime = -1.0;

	int argIndex;
	for (argIndex = 1; argIndex < argc; ++argIndex) {
		if (!strcmp(argv[argIndex], "--endframe")) {
			if (++argIndex >= argc) break;
			endFrameFilename = argv[argIndex];
			if (++argIndex >= argc) break;
			endFrameTime = atof(argv[argIndex]);
		} else if (!strcmp(argv[argIndex], "--silence")) {
			if (++argIndex >= argc) break;
			silence = atof(argv[argIndex]);
		} else break;
	}
	if (argIndex < argc-1) {
		inputFilename = argv[argIndex];
		outputFilename = argv[argIndex + 1];
	}

	if (inputFilename == NULL || outputFilename == NULL || endFrameFilename == NULL || endFrameTime < 0.0) {
		usage(argv[0]);
		exit(1);
	}

	printf("=== cvis ===\n\n"
		"Input audio:  %s\nOutput video: %s\n\n"
		"Initial silence: %f seconds\n\n"
		"End frame: %s\nDisplaying at audio t=%f seconds\n\n============\n\n",
		inputFilename, outputFilename, silence, endFrameFilename, endFrameTime);

	int audioOffset = OUT_AUDIO_OFFSET + (silence * OUT_AUDIO_SAMPLE_RATE);
	int endFrame = (int)(OUT_VIDEO_FRAMERATE * (endFrameTime + ((double)audioOffset / OUT_AUDIO_SAMPLE_RATE)));

	std::vector<double> signal = Decoder(inputFilename).decode();

	auto analyzer = std::make_unique<Analyzer>(FFT_SIZE, FFT_SMOOTHING);
	auto encoder = std::make_unique<Encoder>(outputFilename, audioOffset);

	auto glContext = std::make_shared<GlContext>(WINDOW_WIDTH, WINDOW_HEIGHT, GEN_VIDEO_WIDTH, GEN_VIDEO_HEIGHT);
	auto tetrikScene = std::make_unique<TetrikScene>(glContext);
	auto endFrameScene = std::make_unique<EndFrameScene>(glContext, endFrameFilename);

	bool audioFinished = false;
	while (!audioFinished) {
		if (encoder->nextFrameType() == StreamType_Video) {
			int frame = encoder->videoStream->nextPts;
			if (frame < endFrame) {
				int sample = frame * OUT_AUDIO_SAMPLE_RATE / OUT_VIDEO_FRAMERATE - audioOffset;
				analyzer->analyze(signal.data(), signal.size() / 2, sample);
				tetrikScene->draw(
					(float)frame / OUT_VIDEO_FRAMERATE,
					analyzer->timeResult, FFT_SIZE,
					analyzer->freqResult, FFT_SIZE / 2,
					OUT_AUDIO_SAMPLE_RATE);
			} else {
				endFrameScene->draw();
			}
			glContext->updateWindow();
			glContext->readPixels();
			encoder->writeVideoFrame(glContext->pixels.data());
		} else {
			audioFinished = !encoder->writeAudioFrame(signal.data(), signal.size() / 2);
		}
	}
}

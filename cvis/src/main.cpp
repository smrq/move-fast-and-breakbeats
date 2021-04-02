#include <memory>
#include <array>
#include <string>
#include "analyze.h"
#include "codec/decoding.h"
#include "codec/encoding.h"
#include "constants.h"
#include "deps/gl.h"
#include "glcontext.h"
#include "scenes/endframe.h"
#include "scenes/nwwia.h"
#include "scenes/tetrik.h"

int main_nwwia(const char *outputFilename) {
	const char *inputFilename = "../media/nwwia.flac";
	std::vector<std::string> automationFilenames = {
		"../media/nwwia/track0.tsv",
		"../media/nwwia/track1.tsv",
		"../media/nwwia/track2.tsv",
		"../media/nwwia/track3.tsv",
		"../media/nwwia/track4.tsv",
		"../media/nwwia/track5.tsv",
		"../media/nwwia/track6.tsv",
		"../media/nwwia/track7.tsv",
		"../media/nwwia/track8.tsv",
		"../media/nwwia/track9.tsv",
	};
	const char *endFrameFilename = "../media/endframe.png";
	const double endFrameTime = 100.0;
	const double silence = 0.75;

	int audioOffset = OUT_AUDIO_OFFSET + (silence * OUT_AUDIO_SAMPLE_RATE);
	int endFrame = (int)(OUT_VIDEO_FRAMERATE * (endFrameTime + ((double)audioOffset / OUT_AUDIO_SAMPLE_RATE)));

	std::vector<double> signal = Decoder(inputFilename).decode();

	auto analyzer = std::make_unique<Analyzer>(FFT_SIZE, FFT_SMOOTHING);
	auto encoder = std::make_unique<Encoder>(outputFilename, audioOffset);

	auto glContext = std::make_shared<GlContext>(WINDOW_WIDTH, WINDOW_HEIGHT, GEN_VIDEO_WIDTH, GEN_VIDEO_HEIGHT);
	auto nwwiaScene = std::make_unique<NwwiaScene>(glContext, automationFilenames);
	auto endFrameScene = std::make_unique<EndFrameScene>(glContext, endFrameFilename);

	bool audioFinished = false;
	while (!audioFinished) {
		if (encoder->nextFrameType() == StreamType_Video) {
			int frame = encoder->videoStream->nextPts;
			if (frame < endFrame) {
				int sample = frame * OUT_AUDIO_SAMPLE_RATE / OUT_VIDEO_FRAMERATE - audioOffset;
				analyzer->analyze(signal.data(), signal.size() / 2, sample);
				nwwiaScene->draw(
					(double)frame / OUT_VIDEO_FRAMERATE,
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

	return 0;
}


int main_tetrik(const char *outputFilename) {
	const char *inputFilename = "../media/tetrik.flac";
	const char *endFrameFilename = "../media/endframe.png";
	const double endFrameTime = 452.684;
	const double silence = 0.75;

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
					(double)frame / OUT_VIDEO_FRAMERATE,
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

	return 0;
}

int main(int argc, char **argv) {
	if (argc < 3) {
		printf("usage: %s song outfile.mp4\n", argv[0]);
		exit(1);
	}

	const char *song = argv[1];
	const char *outfile = argv[2];

	if (strcmp(song, "tetrik") == 0) {
		return main_tetrik(outfile);
	}
	if (strcmp(song, "nwwia") == 0) {
		return main_nwwia(outfile);
	}

	return -1;
}

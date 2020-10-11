#include "analyze.h"

Analyzer::Analyzer(unsigned int fftSize, double smoothing)
: fftSize(fftSize), smoothing(smoothing) {
	blackman = (double *)fftw_malloc(sizeof(double) * fftSize);
	double alpha = 0.16;
	double a0 = (1.0 - alpha) / 2.0;
	double a1 = 0.5;
	double a2 = alpha / 2.0;
	for (int i = 0; i < fftSize; ++i) {
		blackman[i] = a0 - (a1 * cos(2.0 * M_PI * i / fftSize)) + (a2 * cos(4.0 * M_PI * i / fftSize));
	}

	fftInput = fftw_alloc_real(fftSize);
	fftOutput = fftw_alloc_complex(fftSize);
	timeResult = fftw_alloc_real(fftSize);
	freqResult = fftw_alloc_real(fftSize / 2);
	lastFreqResult = fftw_alloc_real(fftSize / 2);

	memset(freqResult, 0, sizeof(double) * fftSize / 2);

	plan = fftw_plan_dft_r2c_1d(fftSize, fftInput, fftOutput, FFTW_ESTIMATE);
}

Analyzer::~Analyzer() {
	fftw_destroy_plan(plan);
	fftw_free(fftInput);
	fftw_free(fftOutput);
	fftw_free(timeResult);
	fftw_free(freqResult);
	fftw_free(lastFreqResult);
}

void Analyzer::analyze(double *signal, size_t signalSize, int firstSample) {
	for (int i = 0; i < fftSize; ++i) {
		int sampleIndex = i + firstSample;
		if (sampleIndex >= 0 && sampleIndex < signalSize) {
			timeResult[i] = 0.5 * (signal[2*sampleIndex] + signal[2*sampleIndex + 1]);
		} else {
			timeResult[i] = 0.0;
		}
		fftInput[i] = blackman[i] * timeResult[i];
	}

	fftw_execute(plan);

	double *tmp = lastFreqResult;
	lastFreqResult = freqResult;
	freqResult = tmp;

	for (int i = 0; i < fftSize / 2; ++i) {
		freqResult[i] = sqrt(fftOutput[i][0]*fftOutput[i][0] + fftOutput[i][1]*fftOutput[i][1]) / fftSize;
		freqResult[i] = smoothing * lastFreqResult[i] + (1.0 - smoothing) * freqResult[i];
	}
}

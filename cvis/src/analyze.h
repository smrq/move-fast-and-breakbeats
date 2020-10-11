#include <cmath>
#include <cstring>
#include <fftw3.h>

struct Analyzer {
	unsigned int fftSize;
	double smoothing;

	double *blackman;
	double *fftInput;
	fftw_complex *fftOutput;
	double *timeResult;
	double *freqResult;
	double *lastFreqResult;

	fftw_plan plan;

	Analyzer(unsigned int fftSize, double smoothing);
	void analyze(double *signal, size_t signalSize, int firstSample);
	~Analyzer();
};

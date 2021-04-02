#pragma once
#include <cmath>
#include <cstddef>

void hilbert(int n, int d, int *x, int *y);
double lerp(double value, double in1, double in2, double out1, double out2);
double lerpClamped(double value, double in1, double in2, double out1, double out2);
double getBandDb(double *freqData, size_t freqDataSize, int sampleRate, double freqLow, double freqHigh);

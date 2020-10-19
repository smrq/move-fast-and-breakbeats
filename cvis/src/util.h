#pragma once
#include <cmath>
#include <cstddef>

void hilbert(int n, int d, int *x, int *y);
float lerp(float value, float in1, float in2, float out1, float out2);
float lerpClamped(float value, float in1, float in2, float out1, float out2);
double getBandDb(double *freqData, size_t freqDataSize, int sampleRate, double freqLow, double freqHigh);

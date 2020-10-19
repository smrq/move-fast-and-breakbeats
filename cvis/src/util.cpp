#include "util.h"

void hilbert(int n, int d, int *x, int *y) {
	*x = 0;
	*y = 0;
	int t = d;

	for (int s = 1; s < n; s <<= 1) {
		int rx = 1 & (t >> 1);
		int ry = 1 & (t ^ rx);

		if (ry == 0) {
			if (rx == 1) {
				*x = s - 1 - *x;
				*y = s - 1 - *y;
			}
			int tmp = *x;
			*x = *y;
			*y = tmp;
		}

		*x += s * rx;
		*y += s * ry;
		t = t >> 2;
	}
}

float lerp(float value, float in1, float in2, float out1, float out2) {
	return ((value - in1) / (in2 - in1)) * (out2 - out1) + out1;
}

float lerpClamped(float value, float in1, float in2, float out1, float out2) {
	return lerp((value < in1) ? in1 : (value > in2) ? in2 : value, in1, in2, out1, out2);
}

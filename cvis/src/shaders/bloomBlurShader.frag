in vec2 v_uv;
layout (location = 0) out vec4 f_color;

uniform sampler2D colorTexture;
uniform vec2 texSize;
uniform vec2 direction;

float gaussianPdf(in float x, in float sigma) {
	return 0.39894 * exp(-0.5 * x * x/(sigma * sigma)) / sigma;
}

void main() {
	vec2 invSize = 1.0 / texSize;
	float fSigma = float(BLOOM_KERNEL_SIZE);
	float weightSum = gaussianPdf(0.0, fSigma);
	vec3 diffuseSum = texture(colorTexture, v_uv).rgb * weightSum;
	for (int i = 1; i < BLOOM_KERNEL_SIZE; i++) {
		float x = float(i);
		float w = gaussianPdf(x, fSigma);
		vec2 uvOffset = direction * invSize * x;
		vec3 sample1 = texture(colorTexture, v_uv + uvOffset).rgb;
		vec3 sample2 = texture(colorTexture, v_uv - uvOffset).rgb;
		diffuseSum += (sample1 + sample2) * w;
		weightSum += 2.0 * w;
	}
	f_color = vec4(diffuseSum / weightSum, 1.0);
}

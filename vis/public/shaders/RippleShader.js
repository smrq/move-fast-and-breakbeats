const vertexShader = `
varying vec2 vUv;
void main() {
	vUv = uv;
	gl_Position = projectionMatrix * modelViewMatrix * vec4( position, 1.0 );
}`;

const fragmentShader = `
const int MAX_ITERATIONS = 10;

uniform float aspectRatio;
uniform float time;
uniform float speed;
uniform sampler2D tDiffuse;

uniform float waveWidth;
uniform float waveExp;
uniform float waveOffset;
uniform int waveCount;

varying vec2 vUv;

void main() {
	float t = time * speed;

	vec4 fragColor = texture2D(tDiffuse, vUv);

	vec2 center = vec2(0.5, 0.5 / aspectRatio);
	vec2 texCoord = vUv;
	texCoord.y /= aspectRatio;
	float d = distance(texCoord, center);
	float ww = waveWidth;

	for (int waveIndex = 0; waveIndex < MAX_ITERATIONS; ++waveIndex) {
		if (waveIndex >= waveCount) break;

		if (t > 0.0 && d <= t + ww && d >= t - ww) {
			float fragmentT = abs(d - t);
			float distortion = 1.0 - pow(fragmentT / ww, waveExp);
			vec2 fragmentDirection = normalize(texCoord - center);
			texCoord += ((fragmentDirection * distortion * fragmentT) / (t * d * 40.0));
			texCoord.y *= aspectRatio;
			fragColor = texture2D(tDiffuse, texCoord);
			fragColor += (fragColor * distortion) / (t * d * 40.0);
			break;
		}

		t -= waveOffset;
		ww *= 0.6;
	}

	gl_FragColor = fragColor;
}
`;

export const RippleShader = {
	uniforms: {
		aspectRatio: { value: 1920 / 1080 },
		time: { value: 0.0 },
		speed: { value: 0.4 },
		tDiffuse: { value: null },
		waveWidth: { value: 0.1 },
		waveExp: { value: 0.8 },
		waveOffset: { value: 0.25 },
		waveCount: { value: 4 },
	},
	vertexShader,
	fragmentShader,
};

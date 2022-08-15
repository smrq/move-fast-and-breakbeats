const vertexShader = `
varying vec2 vUv;
void main() {
	vUv = uv;
	gl_Position = projectionMatrix * modelViewMatrix * vec4( position, 1.0 );
}`;

const fragmentShader = `
const float MAX_ITERATIONS = 100.0;

uniform float amount;
uniform float speed;
uniform float time;
uniform sampler2D tDiffuse;

varying vec2 vUv;

float random2d(vec2 n) { 
	return fract(sin(dot(n, vec2(12.9898, 4.1414))) * 43758.5453);
}

float randomRange (in vec2 seed, in float min, in float max) {
	return min + random2d(seed) * (max - min);
}

float insideRange(float v, float bottom, float top) {
	return step(bottom, v) - step(top, v);
}

void main() {
	float time = floor(time * speed * 60.0);

	vec3 outCol = texture2D(tDiffuse, vUv).rgb;

	// Randomly offset slices horizontally
	float maxOffset = amount / 2.0;
	for (float i = 0.0; i < MAX_ITERATIONS; i += 1.0) {
		if (i >= 10.0 * amount) { break; }
		float sliceY = random2d(vec2(time , 2345.0 + float(i)));
		float sliceH = random2d(vec2(time , 9035.0 + float(i))) * 0.25;
		float hOffset = randomRange(vec2(time , 9625.0 + float(i)), -maxOffset, maxOffset);
		vec2 uvOff = vUv;
		uvOff.x += hOffset;
		if (insideRange(vUv.y, sliceY, fract(sliceY+sliceH)) == 1.0 ){
			outCol = texture2D(tDiffuse, uvOff).rgb;
		}
	}
	
	// Do slight offset on one entire channel
	float maxColOffset = amount / 6.0;
	float rnd = random2d(vec2(time, 9545.0));
	vec2 colOffset = vec2(randomRange(vec2(time , 9545.0),-maxColOffset,maxColOffset), 
					   randomRange(vec2(time , 7205.0),-maxColOffset,maxColOffset));
	if (rnd < 0.33) {
		outCol.r = texture2D(tDiffuse, vUv + colOffset).r;
	} else if (rnd < 0.66){
		outCol.g = texture2D(tDiffuse, vUv + colOffset).g;
	} else {
		outCol.b = texture2D(tDiffuse, vUv + colOffset).b;  
	}
	   
	gl_FragColor = vec4(outCol, 1.0);
}
`;

export const RgbGlitchShader = {
	uniforms: {
		amount: { value: 0.15 },
		speed: { value: 0.4 },
		time: { value: 0.0 },
		tDiffuse: { value: null },
	},
	vertexShader,
	fragmentShader,
};

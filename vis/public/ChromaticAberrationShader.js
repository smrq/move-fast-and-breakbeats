const vertexShader = `
varying vec2 vUv;
void main() {
	vUv = uv;
	gl_Position = projectionMatrix * modelViewMatrix * vec4( position, 1.0 );
}`;

const fragmentShader = `
const int levels = 12;
uniform float spread;
uniform sampler2D tDiffuse;

varying vec2 vUv;

void main() {
	vec3 sum = vec3( 0.0 );

	vec2 offset = (vUv - vec2( 0.5 )) * vec2( 1.0, -1.0 );

	for (int i = 0; i < levels; ++i) {
		float t = 2.0 * float(i) / float(levels - 1);
		vec3 slice = vec3( 1.0 - t, 1.0 - abs(t - 1.0), t - 1.0 );
		slice = max(slice, 0.0);
		sum += slice;
		vec2 slice_offset = (t - 1.0) * spread * offset;
		gl_FragColor += vec4(slice * texture2D( tDiffuse, vUv + slice_offset ).rgb, 1.0);
	}
	gl_FragColor /= vec4(sum, 1.0);
}`;

export const ChromaticAberrationShader = {
	uniforms: {
		spread: { value: 0 },
		tDiffuse: { value: null },
	},
	vertexShader,
	fragmentShader,
};

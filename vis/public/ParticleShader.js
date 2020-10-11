import * as THREE from 'https://unpkg.com/three@0.121.1/build/three.module.js';

const vertexShader = `
uniform vec2 resolution;
attribute float scale;

void main() {
	vec4 mvPosition = modelViewMatrix * vec4( position, 1.0 );
	gl_PointSize = scale * (resolution.y / 256.0) * (-300.0/mvPosition.z);
	gl_Position = projectionMatrix * mvPosition;
}`;

const fragmentShader = `
uniform vec3 color;

void main() {
	if ( length( gl_PointCoord - vec2( 0.5, 0.5 ) ) > 0.475 ) discard;
	gl_FragColor = vec4( color, 1.0 );
}
`;

export const ParticleShader = {
	uniforms: {
		color: { value: new THREE.Color(0xffffff) },
		resolution: { value: new THREE.Vector2(256, 256) },
	},
	vertexShader,
	fragmentShader,
};

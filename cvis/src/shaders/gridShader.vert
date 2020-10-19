layout (location = 0) in vec2 positionXZ;
layout (location = 1) in vec2 positionYScale;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;
uniform vec2 resolution;

void main() {
	vec3 position = vec3(positionXZ[0], positionYScale[0], positionXZ[1]);
	float scale = positionYScale[1];
	vec4 mvPosition = view * model * vec4(position, 1.0);
	gl_PointSize = scale * (resolution.y / 256.0) * (-300.0/mvPosition.z);
	gl_Position = projection * mvPosition;
}

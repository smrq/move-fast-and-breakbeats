layout (location = 0) in float positionX;
layout (location = 1) in float positionY;
out vec4 v_color;

uniform vec3 color;

void main() {
	gl_Position = vec4(positionX, positionY, 0.0, 1.0);
	v_color = vec4(color, 1.0);
}

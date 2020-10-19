layout (location = 0) in vec3 position;
layout (location = 1) in vec3 color;
out vec4 v_color;

uniform mat4 projection;
uniform mat4 view;

void main() {
	gl_Position = projection * view * vec4(position, 1.0);
	v_color = vec4(color, 1.0);
}

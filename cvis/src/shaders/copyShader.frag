in vec2 v_uv;
layout (location = 0) out vec4 f_color;

uniform sampler2D screenTexture;

void main() {
	f_color = texture(screenTexture, v_uv);
}

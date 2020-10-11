R"(

#version 330 core

layout (location = 0) in vec2 position;
layout (location = 1) in vec2 uv;
out vec2 v_uv;

void main() {
	v_uv = uv;
	gl_Position = vec4(position, 0.0, 1.0);
}

)", R"(

#version 330 core

in vec2 v_uv;
layout (location = 0) out vec4 f_color;

uniform sampler2D screenTexture;
uniform float focusPosition;
uniform float amount;
uniform float brightness;

void main() {
	vec4 sum = vec4(0.0);
	float vv = abs(focusPosition - v_uv.y) * amount;

	//V blur
	sum += texture(screenTexture, vec2(v_uv.x, v_uv.y - 4.0 * vv)) * 0.051 * brightness;
	sum += texture(screenTexture, vec2(v_uv.x, v_uv.y - 3.0 * vv)) * 0.0918 * brightness;
	sum += texture(screenTexture, vec2(v_uv.x, v_uv.y - 2.0 * vv)) * 0.12245 * brightness;
	sum += texture(screenTexture, vec2(v_uv.x, v_uv.y - 1.0 * vv)) * 0.1531 * brightness;
	sum += texture(screenTexture, vec2(v_uv.x, v_uv.y           )) * 0.1633 * brightness;
	sum += texture(screenTexture, vec2(v_uv.x, v_uv.y + 1.0 * vv)) * 0.1531 * brightness;
	sum += texture(screenTexture, vec2(v_uv.x, v_uv.y + 2.0 * vv)) * 0.12245 * brightness;
	sum += texture(screenTexture, vec2(v_uv.x, v_uv.y + 3.0 * vv)) * 0.0918 * brightness;
	sum += texture(screenTexture, vec2(v_uv.x, v_uv.y + 4.0 * vv)) * 0.051 * brightness;

	//H blur
	sum += texture(screenTexture, vec2(v_uv.x - 4.0 * vv, v_uv.y)) * 0.051 * brightness;
	sum += texture(screenTexture, vec2(v_uv.x - 3.0 * vv, v_uv.y)) * 0.0918 * brightness;
	sum += texture(screenTexture, vec2(v_uv.x - 2.0 * vv, v_uv.y)) * 0.12245 * brightness;
	sum += texture(screenTexture, vec2(v_uv.x - 1.0 * vv, v_uv.y)) * 0.1531 * brightness;
	sum += texture(screenTexture, vec2(v_uv.x,            v_uv.y)) * 0.1633 * brightness;
	sum += texture(screenTexture, vec2(v_uv.x + 1.0 * vv, v_uv.y)) * 0.1531 * brightness;
	sum += texture(screenTexture, vec2(v_uv.x + 2.0 * vv, v_uv.y)) * 0.12245 * brightness;
	sum += texture(screenTexture, vec2(v_uv.x + 3.0 * vv, v_uv.y)) * 0.0918 * brightness;
	sum += texture(screenTexture, vec2(v_uv.x + 4.0 * vv, v_uv.y)) * 0.051 * brightness;

	f_color = sum;
}

)"
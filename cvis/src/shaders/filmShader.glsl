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
uniform float seed;
uniform float noiseLevel;
uniform float scanlineLevel;
uniform float scanlineCount;

const float pi = 3.141592653589793;

highp float rand(const in vec2 uv) {
	const highp float a = 12.9898, b = 78.233, c = 43758.5453;
	highp float dt = dot(uv.xy, vec2(a, b));
	highp float sn = mod(dt, pi);
	return fract(sin(sn) * c);
}

void main() {
	vec4 color = texture(screenTexture, v_uv);
	float dx = rand(v_uv + seed);
	vec2 sc = vec2(sin(v_uv.y * scanlineCount), cos(v_uv.y * scanlineCount));

	vec3 cResult = color.rgb + color.rgb * clamp(0.1 + dx, 0.0, 1.0);
	cResult += color.rgb * vec3(sc.x, sc.y, sc.x) * scanlineLevel;
	cResult = color.rgb + clamp(noiseLevel, 0.0, 1.0) * (cResult - color.rgb);

	f_color = vec4(cResult, color.a);
}

)"
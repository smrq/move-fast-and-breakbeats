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

uniform sampler2D blurTexture0;
uniform sampler2D blurTexture1;
uniform sampler2D blurTexture2;
uniform sampler2D blurTexture3;
uniform sampler2D blurTexture4;

uniform float strength;
uniform float radius;
uniform float factors[5];
uniform vec3 tintColors[5];

float lerpBloomFactor(const in float factor) { 
	float mirrorFactor = 1.2 - factor;
	return mix(factor, mirrorFactor, radius);
}

void main() {
	f_color = strength * (
		lerpBloomFactor(factors[0]) * vec4(tintColors[0], 1.0) * texture(blurTexture0, v_uv) + 
		lerpBloomFactor(factors[1]) * vec4(tintColors[1], 1.0) * texture(blurTexture1, v_uv) + 
		lerpBloomFactor(factors[2]) * vec4(tintColors[2], 1.0) * texture(blurTexture2, v_uv) + 
		lerpBloomFactor(factors[3]) * vec4(tintColors[3], 1.0) * texture(blurTexture3, v_uv) + 
		lerpBloomFactor(factors[4]) * vec4(tintColors[4], 1.0) * texture(blurTexture4, v_uv));
}

)"
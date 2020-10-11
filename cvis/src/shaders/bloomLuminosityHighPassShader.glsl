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

uniform sampler2D tDiffuse;
uniform vec4 defaultColor;
uniform float luminosityThreshold;
uniform float smoothWidth;

void main() {
	vec4 texel = texture(tDiffuse, v_uv);
	vec3 luma = vec3(0.299, 0.587, 0.114);
	float v = dot(texel.xyz, luma);
	vec4 outputColor = vec4(defaultColor);
	float alpha = smoothstep(luminosityThreshold, luminosityThreshold + smoothWidth, v);
	f_color = mix(outputColor, texel, alpha);
}

)"
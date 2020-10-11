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

void main() {
	f_color = texture(screenTexture, v_uv);
}

)"
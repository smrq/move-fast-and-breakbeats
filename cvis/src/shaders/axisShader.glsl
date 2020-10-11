R"(

#version 330 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 color;
out vec4 v_color;

uniform mat4 projection;
uniform mat4 view;

void main() {
	gl_Position = projection * view * vec4(position, 1.0);
	v_color = vec4(color, 1.0);
}

)", R"(

#version 330 core

layout (lines) in;
in vec4 v_color[2];
layout (triangle_strip, max_vertices = 4) out;
out vec4 g_color;

uniform vec3 normal;
uniform float lineWidth;

void main() {
	vec4 start = gl_in[0].gl_Position;
	vec4 end = gl_in[1].gl_Position;
	vec3 halfwidth = normalize(cross(end.xyz - start.xyz, normal)) * lineWidth * 0.5;

	gl_Position = vec4(start.xyz + halfwidth, start.w);
	g_color = v_color[0];
	EmitVertex();
	gl_Position = vec4(start.xyz - halfwidth, start.w);
	EmitVertex();
	gl_Position = vec4(end.xyz + halfwidth, end.w);
	g_color = v_color[1];
	EmitVertex();
	gl_Position = vec4(end.xyz - halfwidth, end.w);
	EmitVertex();
	EndPrimitive();
}

)", R"(

#version 330 core

in vec4 g_color;
layout (location = 0) out vec4 f_color;

void main() {
	f_color = g_color;
}

)"

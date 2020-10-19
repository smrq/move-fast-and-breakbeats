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

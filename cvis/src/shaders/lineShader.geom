layout (lines_adjacency) in;
in vec4 v_color[4];
layout (triangle_strip, max_vertices = 4) out;
out vec4 g_color;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;
uniform float lineWidth;

const float epsilon = 0.0001;
const vec3 normal = vec3(0.0, 0.0, -1.0);

void main() {
	vec3 prev  = gl_in[0].gl_Position.xyz; 
	vec3 start = gl_in[1].gl_Position.xyz;
	vec3 end   = gl_in[2].gl_Position.xyz;
	vec3 next  = gl_in[3].gl_Position.xyz;

	vec3 a = normalize(start - prev);
	vec3 b = normalize(end - start);
	vec3 c = normalize(next - end);

	vec3 x = cross(b, normal);

	vec3 v1 = length(b - a) > epsilon ?
		normalize(b - a) * lineWidth :
		x;

	vec3 v2 = length(c - b) > epsilon ?
		normalize(c - b) * lineWidth :
		x;

	float s1 = sign(dot(v1, x));
	float s2 = sign(dot(v2, x));

	gl_Position = projection * view * model * vec4(start - s1*v1, 1.0);
	g_color = v_color[1];
	EmitVertex();
	gl_Position = projection * view * model * vec4(start + s1*v1, 1.0);
	EmitVertex();
	gl_Position = projection * view * model * vec4(end - s2*v2, 1.0);
	g_color = v_color[2];
	EmitVertex();
	gl_Position = projection * view * model * vec4(end + s2*v2, 1.0);
	EmitVertex();
	EndPrimitive();
}


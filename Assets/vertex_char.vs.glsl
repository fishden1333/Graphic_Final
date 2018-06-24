#version 410

layout(location = 0) in vec3 iv3vertex;
layout(location = 1) in vec3 iv3normal;
layout(location = 2) in vec2 iv2color;

uniform mat4 um4mvp;
uniform mat4 uniform_mv;
uniform vec3 light_pos = vec3(100.0, 100.0, 100.0);

out VertexData
{
    vec3 N; // eye space normal
    vec3 L; // eye space light vector
    vec3 V; // eye space view vector
    vec3 vv3color;
    vec2 vv2color;
    vec4 viewSpace_coord;
} vertexData;

void main()
{
	vec4 P = uniform_mv * vec4(iv3vertex, 1.0);
    vertexData.N = transpose(inverse(mat3(uniform_mv))) * iv3normal;
    vertexData.L = light_pos - P.xyz;
    vertexData.V = -P.xyz;

	gl_Position = um4mvp * vec4(iv3vertex, 1);
	vertexData.vv3color = iv3normal;
	vertexData.vv2color = iv2color;
    vertexData.viewSpace_coord = uniform_mv * vec4(iv3vertex, 1.0);
}





#version 410

layout(location = 0) in vec3 iv3vertex;
layout(location = 1) in vec2 iv2tex_coord;
layout(location = 2) in vec3 iv3normal;

uniform mat4 um4mv;
uniform mat4 um4p;
uniform vec3 light_pos = vec3(100.0, 100.0, 100.0);

out VertexData
{
    vec3 N; // eye space normal
    vec3 L; // eye space light vector
    vec3 V; // eye space view vector
    vec2 texcoord;
    vec3 normal;
    vec4 viewSpace_coord;
} vertexData;

void main()
{
    vec4 P = um4mv * vec4(iv3vertex, 1.0);
    vertexData.N = transpose(inverse(mat3(um4mv))) * iv3normal;
    vertexData.L = light_pos - P.xyz;
    vertexData.V = -P.xyz;
    
    gl_Position = um4p * um4mv * vec4(iv3vertex, 1.0);
    vertexData.texcoord = iv2tex_coord;
    vertexData.normal = iv3normal;
    vertexData.viewSpace_coord = um4mv * vec4(iv3vertex, 1.0);
}


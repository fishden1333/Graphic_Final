#version 410 core

in VertexData
{
    vec3 N; // eye space normal
    vec3 L; // eye space light vector
    vec3 V; // eye space halfway vector
    vec3 vv3color;
    vec2 vv2color;
    vec4 viewSpace_coord;
} vertexData;

out vec4 fragColor;

uniform int fog2;
uniform int mode;
uniform sampler2D tex_object;
uniform vec3 specular_albedo = vec3(1.0);
uniform float specular_power = 3000.0;
const vec4 fogColor = vec4(0.2, 0.3, 0.2, 1.0);
float fogFactor;
float fogDensity = 0.3;

void main() 
{
	vec3 N = normalize(vertexData.N);
    vec3 L = normalize(vertexData.L);
    vec3 V = normalize(vertexData.V);
    vec3 H = normalize(L + V);

	vec3 ambient;
	vec3 specular = pow(max(dot(N, H), 0.0), specular_power) * specular_albedo;
	vec3 diffuse;
	if (mode == 0)
	{
		ambient = vertexData.vv3color;
		diffuse = max(dot(N, L), 0.0) * vertexData.vv3color;
	}
	else
	{
		ambient = texture(tex_object, vertexData.vv2color).rgb;
		diffuse = max(dot(N, L), 0.0) * texture(tex_object, vertexData.vv2color).rgb;
	}
	vec4 color = vec4(ambient * 0.4 + diffuse * 0.8 + specular * 0.2, 1.0);
    
    float dist = length(vertexData.viewSpace_coord);
    fogFactor = 1.0 / exp(dist * fogDensity * dist * fogDensity);
    fogFactor = clamp(fogFactor, 0.0, 1.0);
    
    if (fog2 == 1)
    {
        fragColor = mix(fogColor, color, fogFactor);
    }
    else
    {
        fragColor = color;
    }
}




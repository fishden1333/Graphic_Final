#version 410

layout(location = 0) out vec4 fragColor;

uniform mat4 um4mv;
uniform mat4 um4p;

in VertexData
{
    vec3 N; // eye space normal
    vec3 L; // eye space light vector
    vec3 V; // eye space halfway vector
    vec2 texcoord;
    vec3 normal;
    vec4 viewSpace_coord;
} vertexData;

uniform int fog1;
uniform sampler2D tex;
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
    
    vec3 texColor = texture(tex,vertexData.texcoord).rgb;
    vec3 diffuse = max(dot(N, L), 0.0) * texColor;
    vec3 specular = pow(max(dot(N, H), 0.0), specular_power) * specular_albedo;
    
    float dist = length(vertexData.viewSpace_coord);
    fogFactor = 1.0 / exp(dist * fogDensity * dist * fogDensity);
    fogFactor = clamp(fogFactor, 0.0, 1.0);
    
    vec4 color = vec4(texColor * 0.4 + diffuse * 0.8 + specular * 0.2, 1.0);
    
    if (fog1 == 1)
    {
        fragColor = mix(fogColor, color, fogFactor);
    }
    else
    {
        fragColor = color;
    }
}


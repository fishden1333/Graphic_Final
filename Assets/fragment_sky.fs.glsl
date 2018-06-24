#version 410 core

uniform samplerCube tex_cubemap;

in VS_OUT
{
    vec3    tc;
} fs_in;

layout (location = 0) out vec4 color;

void main(void)
{
    vec3 sky_color = texture(tex_cubemap, fs_in.tc).rgb;
    color = vec4(sky_color.r, sky_color.g, sky_color.b, 1.0);
}


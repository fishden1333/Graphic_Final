#version 410 core

layout(location = 0) in vec2 position;
layout(location = 1) in vec2 tex_coord;

out VS_OUT
{
    vec2 texcoord;
} vs_out;

void main()
{
	gl_Position = vec4(position, 1.0, 1.0);
    vs_out.texcoord = tex_coord;
}

#version 330 core

uniform sampler2D u_texture;
uniform lowp vec4 u_color;

out vec4 color;

in vec2 v_texcoord;

void main()
{
	color = texture(u_texture, v_texcoord) * u_color;
}
#version 450

out vec4 FragColor;

uniform sampler2D u_texture;
in vec2 v_uvs;

void main()
{
	FragColor = texture(u_texture, v_uvs);
}
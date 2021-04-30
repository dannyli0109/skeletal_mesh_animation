#version 450

out vec4 FragColor;
in vec2 v_uvs;
uniform sampler2D u_colourTexture;

void main()
{
	vec4 color = texture(u_colourTexture, v_uvs);
	FragColor = color;
}
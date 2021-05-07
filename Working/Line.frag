#version 450

out vec4 f_color;

in vec4 v_color;

void main()
{
	f_color = v_color;
}
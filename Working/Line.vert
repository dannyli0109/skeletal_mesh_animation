#version 450

layout (location = 0) in vec3 a_position;
layout (location = 1) in vec4 a_color;

out vec4 v_colour;

uniform mat4 u_projectionMatrix;

void main()
{
	v_colour = a_color;
	gl_Position = u_projectionMatrix * vec4(a_position, 1);
}
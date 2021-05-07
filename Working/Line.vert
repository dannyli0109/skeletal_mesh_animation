#version 450

layout (location = 0) in vec3 a_position;
layout (location = 1) in vec4 a_color;

out vec4 v_color;

uniform mat4 u_projectionMatrix;
uniform mat4 u_viewMatrix;

void main()
{
	v_color = a_color;
	gl_Position =  (u_projectionMatrix * u_viewMatrix) * vec4(a_position, 1);
}
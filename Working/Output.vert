#version 450
layout (location = 0) in vec2 a_position;
layout (location = 5) in vec2 a_uvs;

uniform float u_width;
uniform float u_height;
uniform float u_top;
uniform float u_left;

out vec2 v_uvs;
void main()
{
	v_uvs = vec2(a_uvs.x, 1.0 - a_uvs.y);
	/*
	float offsetLeft = (2.0 - u_width * 2.0) / 2.0;
	float paddingLeft = u_left * 2.0 * u_width;
	
	float offsetTop = -(2.0 - u_height * 2.0) / 2.0;
	float paddingTop = -u_top * 2.0 * u_height;
	vec4 position = vec4(a_position.x * u_width * 2.0 - offsetLeft + paddingLeft, a_position.y * u_height * 2.0 - offsetTop + paddingTop, 0, 1);
	*/
	vec4 position = vec4(a_position * 2.0, 0, 1);
	gl_Position = position;
}
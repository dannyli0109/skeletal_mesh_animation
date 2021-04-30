#version 450
layout (location = 0) in vec2 a_position;
layout (location = 5) in vec2 a_uvs;

out vec2 v_uvs;
void main()
{
	v_uvs = vec2(a_uvs.x, 1.0 - a_uvs.y);
	gl_Position = vec4(a_position * 2.0, 0, 1);
}
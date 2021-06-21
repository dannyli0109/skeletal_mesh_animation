#version 450

out vec4 FragColor;

in vec4 v_normal;

void main()
{
	vec4 normal = normalize(v_normal);
	FragColor = vec4(normal.xyz, 1);
}
#version 450

out vec4 FragColor;

uniform sampler2D u_normalTexture;
in vec2 v_uvs;
in mat3 v_tbn;

void main()
{
	vec4 normalTap = texture(u_normalTexture, v_uvs);
	vec3 mapNormal = normalTap.xyz * 2 - 1;
	vec4 normal = normalize(vec4(v_tbn * mapNormal, 0));
	FragColor = vec4(normal.xyz, 1);
}
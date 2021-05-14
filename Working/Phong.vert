#version 450

layout (location = 0) in vec3 a_position;
layout (location = 1) in vec3 a_normal;
layout (location = 2) in vec3 a_tangent;
layout (location = 3) in vec3 a_bitangent;
layout (location = 4) in vec3 a_color;
layout (location = 5) in vec2 a_uvs;
layout (location = 6) in ivec4 a_boneIds;
layout (location = 7) in vec4 a_weights;

out vec3 v_color;
out vec2 v_uvs;
out vec4 v_position;

out mat3 v_tbn;

uniform mat4 u_projectionMatrix;
uniform mat4 u_viewMatrix;
uniform mat4 u_modelMatrix;

const int MAX_BONES = 100;
const int MAX_BONE_INFLUENCE = 4;
uniform mat4 u_boneTransforms[MAX_BONES];

void main()
{
	v_color = a_color;
	v_uvs = a_uvs;
	
	mat4 boneTransform = mat4(0.0);
	boneTransform += u_boneTransforms[a_boneIds[0]] * a_weights[0];
	boneTransform += u_boneTransforms[a_boneIds[1]] * a_weights[1];
	boneTransform += u_boneTransforms[a_boneIds[2]] * a_weights[2];
	boneTransform += u_boneTransforms[a_boneIds[3]] * a_weights[3];
	
	if (a_weights[0] == 0.0)
	{
		boneTransform = mat4(1.0);
	}
	
	vec4 pos = boneTransform * vec4(a_position, 1.0);
	vec4 normal = boneTransform * vec4(a_normal, 0.0);
	
	vec3 t = normalize(vec3(u_modelMatrix * vec4(a_tangent, 0.0)));
	vec3 b = normalize(vec3(u_modelMatrix * vec4(a_bitangent, 0.0)));
	vec3 n = normalize(vec3(u_modelMatrix * normal));
	
	v_tbn = mat3(
		t, b, n
	);
	
	v_position = u_modelMatrix * pos;
	gl_Position = (u_projectionMatrix * u_viewMatrix * u_modelMatrix) * pos;
}
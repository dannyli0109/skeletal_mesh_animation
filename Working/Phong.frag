#version 450

out vec4 f_color;

uniform sampler2D u_diffuseTexture;
uniform sampler2D u_normalTexture;
uniform sampler2D u_specularTexture;

// color
in vec3 v_color;
// texture
in vec2 v_uvs;
// diffuse
in vec4 v_position;
// normal
in mat3 v_tbn;

// point light
uniform vec3 u_lightPositions[16];
uniform vec3 u_lightColors[16];
uniform float u_lightIntensities[16];
uniform int u_lightCount;

// ambient light
uniform vec3 u_ambientLightIntensity;
uniform vec3 u_ka;
// uniform vec3 lightPos;
// uniform vec3 lightIntensity;

// diffuse coefficient
uniform vec3 u_kd;
// specular coefficient
uniform vec3 u_ks;
uniform vec3 u_cameraPos;
uniform float u_specularPower;


vec4 calculateNormal()
{
	vec4 normalTap = texture(u_normalTexture, v_uvs);
	vec3 mapNormal = normalTap.xyz * 2 - 1;
	return normalize(vec4(v_tbn * mapNormal, 0));
}

vec4 calculateAmbientLighting()
{
	return vec4(u_ambientLightIntensity, 1.0) * vec4(u_ka, 1.0);
}

vec4 calculateDiffuseLighting(vec4 toLight, vec4 normal, float attenuation, vec3 lightIntensity)
{
	vec4 diffuseColor = texture(u_diffuseTexture, v_uvs);
	float diffuseLevel = max(0.0, dot(toLight, normal));
	vec4 diffuseTerm = vec4(u_kd, 1.0) * vec4(lightIntensity, 1) * attenuation * diffuseLevel;
	return diffuseTerm * diffuseColor;
}

vec4 calculateSpecularLighting(vec4 toLight, vec4 normal, float attenuation, vec3 lightIntensity)
{
	vec4 specularColor = texture(u_specularTexture, v_uvs);
	vec4 toCamera = normalize(vec4(u_cameraPos, 1.0) - v_position);
	vec4 reflection = reflect(-toLight, normal);
	float specularLevel = pow(max(0.0, dot(toCamera, reflection)), u_specularPower);
	vec4 specularTerm = vec4(u_ks, 1.0) * vec4(lightIntensity, 1) * attenuation * specularLevel;
	return specularTerm * specularColor;
}

void main()
{
	
	vec4 normal = calculateNormal();
	vec4 result = {0.0, 0.0, 0.0, 0.0};
	for(int i = 0; i < u_lightCount; i++)
	{
		vec4 toLight = normalize(vec4(u_lightPositions[i], 1.0) - v_position);
		float lightDistSquared = dot(vec4(u_lightPositions[i], 1.0) - v_position, vec4(u_lightPositions[i], 1.0) - v_position);
		float attenuation = 1.0 / lightDistSquared;
		
		vec4 diffuseTerm = calculateDiffuseLighting(toLight, normal, attenuation, u_lightIntensities[i] * u_lightColors[i]);
		vec4 specularTerm = calculateSpecularLighting(toLight, normal, attenuation, u_lightIntensities[i] * u_lightColors[i]);
		
		result += diffuseTerm;
		result += specularTerm;
	}
	vec4 ambientTerm = calculateAmbientLighting();
	result += ambientTerm;
	
	f_color = result;
}
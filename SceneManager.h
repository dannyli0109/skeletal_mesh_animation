#pragma once

struct PointLights
{
	std::vector<glm::vec3> positions;
	std::vector<glm::vec3> colors;
	std::vector<float> intensities;
	int count;
};

struct Models
{
	std::vector<Mesh> meshes;
	std::vector<glm::mat4> transforms;
	std::vector<Material> materials;
	std::vector<Animation> animations;
	int count;
};

struct AmbientLight
{
	glm::vec3 color;
	float intensity;
};

struct Scene
{
	PointLights pointLights;
	Camera camera;
	Models models;
	AmbientLight ambientLight;
};

static void AddPointLight(Scene* scene, glm::vec3 lightPosition, glm::vec3 lightColor, float lightIntenisty)
{
	scene->pointLights.positions.push_back(lightPosition);
	scene->pointLights.colors.push_back(lightColor);
	scene->pointLights.intensities.push_back(lightIntenisty);
	scene->pointLights.count++;
}

static std::pair<glm::vec3, glm::vec3> GetAnimationBoundingVolume(Mesh* mesh, Animation* animation, int frames)
{
	float frameTime = 0;
	float timeIncrement = animation->duration / frames;

	float minX = FLT_MAX;
	float maxX = FLT_MIN;

	float minY = FLT_MAX;
	float maxY = FLT_MIN;

	float minZ = FLT_MAX;
	float maxZ = FLT_MIN;

	for (int i = 0; i < frames; i++)
	{
		glm::mat4 parentTransform(1.0f);
		GetPose(*animation, animation->skeleton, frameTime, parentTransform);
		std::vector<glm::mat4> boneTransforms = animation->currentPose;
		for (int j = 0; j < mesh->vertices.size(); j++)
		{
			VertexData vertex = mesh->vertices[j];
			glm::mat4 boneTransform = glm::mat4(0.0f);
			for (int k = 0; k < 4; k++)
			{
				boneTransform += boneTransforms[vertex.mesh.boneIDs[k]] * vertex.mesh.weights[k];
			}

			if (vertex.mesh.weights[0] == 0)
			{
				boneTransform = glm::mat4(1.0f);
			}

			glm::vec4 pos = boneTransform * glm::vec4(vertex.mesh.position, 1.0f);
			glm::vec4 finalPos = pos;
			if (finalPos.x > maxX) maxX = finalPos.x;
			if (finalPos.x < minX) minX = finalPos.x;
			if (finalPos.y > maxY) maxY = finalPos.y;
			if (finalPos.y < minY) minY = finalPos.y;
			if (finalPos.z > maxZ) maxZ = finalPos.z;
			if (finalPos.z < minZ) minZ = finalPos.z;
		}
		frameTime += timeIncrement;
	}
	return { {minX, minY, minZ}, {maxX, maxY, maxZ} };
}

static void AddModel(
	Scene* scene, Mesh mesh, 
	glm::mat4 transform,
	Material material,
	Animation animation
)
{
	scene->models.meshes.push_back(mesh);
	scene->models.transforms.push_back(transform);
	scene->models.materials.push_back(material);
	scene->models.animations.push_back(animation);
	scene->models.count++;


}

static void UpdatePointLights(ShaderProgram* shaderProgram, PointLights& pointLights)
{
	if (pointLights.count == 0) return;
	SetUniform(shaderProgram, "u_lightPositions", pointLights.positions[0], pointLights.count);
	SetUniform(shaderProgram, "u_lightColors", pointLights.colors[0], pointLights.count);
	SetUniform(shaderProgram, "u_lightIntensities", pointLights.intensities[0], pointLights.count);
	SetUniform(shaderProgram, "u_lightCount", (int)pointLights.count);
}

static void UpdateCamera(ShaderProgram* shaderProgram, Camera& camera)
{
	glm::vec3 cameraPos = camera.position;
	glm::mat4 projectionMatrix = GetProjectionMatrix(&camera);
	glm::mat4 viewMatrix = GetViewMatrix(&camera);

	SetUniform(shaderProgram, "u_projectionMatrix", projectionMatrix);
	SetUniform(shaderProgram, "u_viewMatrix", viewMatrix);
	SetUniform(shaderProgram, "u_cameraPos", cameraPos);
}

static void UpdateMaterial(ShaderProgram* shaderProgram, Material& material)
{
	switch (material.type)
	{
	case 0:
	{
		SetUniform(shaderProgram, "u_ka", material.phong.ka);
		SetUniform(shaderProgram, "u_kd", material.phong.kd);
		SetUniform(shaderProgram, "u_ks", material.phong.ks);
		SetUniform(shaderProgram, "u_ke", material.phong.ke);
		SetUniform(shaderProgram, "u_specularPower", material.phong.specularPower);

		BindTexture(&material.phong.diffuseTexture, 0);
		BindTexture(&material.phong.normalTexture, 1);
		BindTexture(&material.phong.specularTexture, 2);
		BindTexture(&material.phong.emissionTexture, 3);
		break;
	}
	case 1:
	{
		SetUniform(shaderProgram, "u_color", material.color.color);
		break;
	}
	default:
		break;
	}
}

static void UpdateModels(ShaderProgram* shaderProgram, Models& models, float elapsedTime)
{
	for (int i = 0; i < models.count; i++)
	{
		UpdateMaterial(shaderProgram, models.materials[i]);
		SetUniform(shaderProgram, "u_modelMatrix", models.transforms[i]);
		glm::mat4 parentTransform(1.0f);
		GetPose(models.animations[i], models.animations[i].skeleton, elapsedTime, parentTransform);
		SetUniform(shaderProgram, "u_boneTransforms", models.animations[i].currentPose[0], models.animations[i].currentPose.size());
	}
}

static void UpdateAmbientLight(ShaderProgram* shaderProgram, AmbientLight& ambientLight)
{
	SetUniform(shaderProgram, "u_ambientLightIntensity", ambientLight.color * ambientLight.intensity);
}

static void UpdateScene(ShaderProgram* shaderProgram, Scene& scene, float elapsedTime)
{
	UpdatePointLights(shaderProgram, scene.pointLights);
	UpdateCamera(shaderProgram, scene.camera);
	UpdateModels(shaderProgram, scene.models, elapsedTime);
	UpdateAmbientLight(shaderProgram, scene.ambientLight);
}


static void RenderModels(ShaderProgram* shaderProgram, Models& models)
{
	glUseProgram(shaderProgram->shaderProgram);
	for (int i = 0; i < models.count; i++)
	{
		DrawMesh(&models.meshes[i]);
	}
}

static void RenderLigths(ShaderProgram* shaderProgram, Resource& resourceManager, Scene& scene)
{
	glUseProgram(shaderProgram->shaderProgram);
	glm::mat4 projectionMatrix = GetProjectionMatrix(&scene.camera);
	glm::mat4 viewMatrix = GetViewMatrix(&scene.camera);

	SetUniform(shaderProgram, "u_projectionMatrix", projectionMatrix);
	SetUniform(shaderProgram, "u_viewMatrix", viewMatrix);

	for (int i = 0; i < scene.pointLights.count; i++)
	{
		Material material = {};
		material.color.color = scene.pointLights.colors[i];
		material.type = 1;
		UpdateMaterial(shaderProgram, material);
		glm::mat4 modelMatrix = glm::translate(glm::mat4(1.0f), scene.pointLights.positions[i]);
		SetUniform(shaderProgram, "u_modelMatrix", modelMatrix);
		DrawMesh(&resourceManager.meshes.sphere);
	}
}

static void InitScene(Scene* scene, Resource* resource, Window* window)
{
	{
		Camera camera = {};
		camera.position = { 0, 10000.0f, 30000.0f };
		camera.up = { 0, 1.0f, 0 };
		camera.theta = glm::radians(270.0f);
		camera.phi = glm::radians(0.0f);

		camera.fovY = glm::radians(45.0f);
		camera.aspect = (float)window->width / (float)window->height;
		camera.near = 1.0f;
		camera.far = 100000.0f;
		scene->camera = camera;
	}

	{
		glm::mat4 modelMatrix = glm::scale(glm::mat4(1.0f), { 1.0f, 1.0f, 1.0f });
		AddModel(scene, resource->meshes.vampire, modelMatrix,
			resource->materials.vampirePhongMaterial,
			resource->animations.vampireAnimation
		);
	}

	{
		AmbientLight ambientLight = {};
		ambientLight.color = { 1.0f, 1.0f, 1.0f };
		ambientLight.intensity = 1.0f;
		scene->ambientLight = ambientLight;
	}

	{
		glm::vec3 lightPosition = { 0.0f, 100.0f, 100.0f };
		glm::vec3 lightColor = { 1.0f, 1.0f, 1.0f };
		float lightIntensity = 5000.0f;
		//AddPointLight(&scene, lightPosition, lightColor, lightIntensity);
	}
}

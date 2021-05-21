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
	std::vector<std::string> names;
	std::vector<Mesh*> meshes;
	std::vector<glm::vec3> positions;
	std::vector<glm::vec3> rotations;
	std::vector<glm::vec3> scales;
	//std::vector<glm::mat4> transforms;
	std::vector<Material*> materials;
	std::vector<Animation*> animations;
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
	Camera2D camera2D;
};

static void AddPointLight(Scene* scene, glm::vec3 lightPosition, glm::vec3 lightColor, float lightIntenisty)
{
	scene->pointLights.positions.push_back(lightPosition);
	scene->pointLights.colors.push_back(lightColor);
	scene->pointLights.intensities.push_back(lightIntenisty);
	scene->pointLights.count++;
}

static std::pair<glm::vec3, glm::vec3> GetAnimationBoundingVolume(Mesh* mesh, Animation* animation, glm::mat4 modelMatrix, int frames)
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
		GetPose(animation, animation->skeleton, frameTime, parentTransform);
		std::vector<glm::mat4> boneTransforms = animation->currentPose;
		for (int j = 0; j < mesh->vertices.size(); j++)
		{
			VertexData vertex = mesh->vertices[j];
			glm::mat4 boneTransform = glm::mat4(0.0f);
			for (int k = 0; k < 4; k++)
			{
				boneTransform += boneTransforms[vertex.animated.boneIDs[k]] * vertex.animated.weights[k];
			}

			if (vertex.animated.weights[0] == 0)
			{
				boneTransform = glm::mat4(1.0f);
			}

			glm::vec4 pos = boneTransform * glm::vec4(vertex.mesh.position, 1.0f);
			glm::vec4 finalPos = modelMatrix * pos;
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

static std::pair<glm::vec3, glm::vec3> GetBoundingVolume(Mesh* mesh, glm::mat4 modelMatrix)
{
	float minX = FLT_MAX;
	float maxX = FLT_MIN;

	float minY = FLT_MAX;
	float maxY = FLT_MIN;

	float minZ = FLT_MAX;
	float maxZ = FLT_MIN;
	for (int i = 0; i < mesh->vertices.size(); i++)
	{
		VertexData vertex = mesh->vertices[i];
		glm::vec4 pos = glm::vec4(vertex.mesh.position, 1.0f);
		glm::vec4 finalPos = modelMatrix * pos;
		if (finalPos.x > maxX) maxX = finalPos.x;
		if (finalPos.x < minX) minX = finalPos.x;
		if (finalPos.y > maxY) maxY = finalPos.y;
		if (finalPos.y < minY) minY = finalPos.y;
		if (finalPos.z > maxZ) maxZ = finalPos.z;
		if (finalPos.z < minZ) minZ = finalPos.z;
	}
	return { {minX, minY, minZ}, {maxX, maxY, maxZ} };
}

static void AddModel(
	Scene* scene,
	std::string name,
	Mesh* mesh,
	//glm::mat4 transform,
	glm::vec3 position,
	glm::vec3 rotation,
	glm::vec3 scale,
	Material* material,
	Animation* animation
)
{
	scene->models.names.push_back(name);
	scene->models.meshes.push_back(mesh);
	//scene->models.transforms.push_back(transform);
	scene->models.positions.push_back(position);
	scene->models.rotations.push_back(rotation);
	scene->models.scales.push_back(scale);

	scene->models.materials.push_back(material);
	scene->models.animations.push_back(animation);
	scene->models.count++;


}

static glm::mat4 GetModelMatrix(glm::vec3 position, glm::vec3 rotation, glm::vec3 scale)
{
	glm::mat4 modelMatrix(1.0f);
	modelMatrix = glm::scale(modelMatrix, scale);

	modelMatrix = glm::rotate(modelMatrix, rotation.z, glm::vec3(0, 0, 1));
	modelMatrix = glm::rotate(modelMatrix, rotation.x, glm::vec3(1, 0, 0));
	modelMatrix = glm::rotate(modelMatrix, rotation.y, glm::vec3(0, 1, 0));

	modelMatrix = glm::translate(modelMatrix, position);
	return modelMatrix;
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

static void UpdateMaterial(Material* material)
{
	ShaderProgram* shaderProgram = material->shaderProgram;
	switch (material->type)
	{
	case 0:
	{
		SetUniform(shaderProgram, "u_ka", material->phong.ka);
		SetUniform(shaderProgram, "u_kd", material->phong.kd);
		SetUniform(shaderProgram, "u_ks", material->phong.ks);
		SetUniform(shaderProgram, "u_ke", material->phong.ke);
		SetUniform(shaderProgram, "u_specularPower", material->phong.specularPower);

		BindTexture(material->phong.diffuseTexture, 0);
		BindTexture(material->phong.normalTexture, 1);
		BindTexture(material->phong.specularTexture, 2);
		BindTexture(material->phong.emissionTexture, 3);
		break;
	}
	case 1:
	{
		SetUniform(shaderProgram, "u_color", material->color.color);
		break;
	}
	case 2:
	{
		BindTexture(material->normal.texture, 0);
		break;
	}
	case 3:
	{
		BindTexture(material->diffuse.texture, 0);
		break;
	}
	case 4:
	{
		BindTexture(material->specular.texture, 0);
		break;
	}
	case 5:
	{
		BindTexture(material->emission.texture, 0);
		break;
	}
	default:
		break;
	}
}

static void UpdateModels(Models& models, float elapsedTime)
{
	for (int i = 0; i < models.count; i++)
	{
		ShaderProgram* shader = models.materials[i]->shaderProgram;
		UpdateMaterial(models.materials[i]);
		glm::mat4 modelMatrix = GetModelMatrix(models.positions[i], models.rotations[i], models.scales[i]);
		SetUniform(shader, "u_modelMatrix", modelMatrix);
		if (models.animations[i])
		{
			glm::mat4 parentTransform(1.0f);
			GetPose(models.animations[i], models.animations[i]->skeleton, elapsedTime, parentTransform);
			SetUniform(shader, "u_boneTransforms", models.animations[i]->currentPose[0], models.animations[i]->currentPose.size());
			SetUniform(shader, "u_animated", true);
		}
		else {
			SetUniform(shader, "u_animated", false);
		}
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
	//UpdateModels(scene.models, elapsedTime);
	UpdateAmbientLight(shaderProgram, scene.ambientLight);
}


static void RenderModels(Models& models)
{
	for (int i = 0; i < models.count; i++)
	{
		glUseProgram(models.materials[i]->shaderProgram->shaderProgram);
		DrawMesh(models.meshes[i]);
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
		material.shaderProgram = shaderProgram;
		UpdateMaterial(&material);
		glm::mat4 modelMatrix = glm::translate(glm::mat4(1.0f), scene.pointLights.positions[i]);
		modelMatrix = glm::scale(modelMatrix, { 100, 100, 100 });
		SetUniform(shaderProgram, "u_modelMatrix", modelMatrix);
		DrawMesh(&resourceManager.meshes[SPHERE_MESH]);
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
		Camera2D camera2D = {};
		camera2D.position = { 0, 0 };
		camera2D.windowSize = { 1280.0f, 720.0f };
		camera2D.zoom = 72.0f;
		camera2D.zoomSpeed = 5.0f;
		camera2D.aspect = 1280.0f / 720.0f;
		scene->camera2D = camera2D;
	}

	{
		/*glm::mat4 modelMatrix = glm::scale(glm::mat4(1.0f), { 1.0f, 1.0f, 1.0f });*/
		glm::vec3 position = { 0, 0, 0 };
		glm::vec3 rotation = { 0, 0, 0 };
		glm::vec3 scale = { 1.0f, 1.0f, 1.0f };
		AddModel(scene,
			"Vampire",
			&resource->meshes[VAMPIRE_MESH], 
			position, rotation, scale,
			&resource->materials[VAMPIRE_PHONG_MATERIAL],
			&resource->animations[VAMPIRE_ANIMATION]
			//nullptr
		);
	}

	{
		AmbientLight ambientLight = {};
		ambientLight.color = { 1.0f, 1.0f, 1.0f };
		ambientLight.intensity = 1.0f;
		scene->ambientLight = ambientLight;
	}

	{
		glm::vec3 lightPosition = { 0.0f, 10000.0f, 15000.0f };
		glm::vec3 lightColor = { 1.0f, 1.0f, 1.0f };
		float lightIntensity = 50000000.0f;
		//AddPointLight(scene, lightPosition, lightColor, lightIntensity);
	}
}



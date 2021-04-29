#pragma once

struct PointLight
{
	glm::vec3 position;
	glm::vec3 color;
	float intensity;
};

struct SceneManager
{
	std::vector<PointLight> pointlights;
};

static void AddPointLight(SceneManager* sceneManager, PointLight pointLight)
{
	sceneManager->pointlights.push_back(pointLight);
}

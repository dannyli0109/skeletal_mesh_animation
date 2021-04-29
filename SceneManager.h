#pragma once

struct PointLights
{
	std::vector<glm::vec3> positions;
	std::vector<glm::vec3> colors;
	std::vector<float> intensities;
	int lightCount;
};

struct SceneManager
{
	PointLights pointlights;
};

static void AddPointLight(SceneManager* sceneManager, glm::vec3 lightPosition, glm::vec3 lightColor, float lightIntenisty)
{
	sceneManager->pointlights.positions.push_back(lightPosition);
	sceneManager->pointlights.colors.push_back(lightColor);
	sceneManager->pointlights.intensities.push_back(lightIntenisty);
	sceneManager->pointlights.lightCount++;
}

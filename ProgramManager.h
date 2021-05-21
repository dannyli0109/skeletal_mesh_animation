#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <filesystem>
#include <vector>
#include <unordered_map>
#include <filesystem>
#include "Graphics.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#define STB_IMAGE_STATIC
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_STATIC
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include "Utils.h"
#include "InputManager.h"

#include "Renderer.h"
#include "ResourceManager.h"
#include "SceneManager.h"
#include "GUI.h"
#include "lib/ImGuiFileDialog/ImGuiFileDialog.h"

class ProgramManager
{
public:
	ProgramManager() = default;
	int Init();
	void Update();
	void Destroy();

private:
	void CaptureAnimationFrames(int numFrames);
	void CaptureStaticFrame();
	void RenderBoundingVolume();
	void RenderSceneWindow();
	void RenderSpriteWindow();
	void RenderSidePannel();
	void SnapCameraToBoundingVolume(std::pair<glm::vec3, glm::vec3> volume);
	void RenderResourcePannel();
	void RenderSceneHierarchy();
	void RenderModelDetailPannel();
	std::string GetNextUIID();
	std::string AppendNextUIID(std::string input);
	//void LoadResourceGUI(std::string title, bool& isLoading);

private:
	Window window;
	// GLFWwindow* window;
	Resource resource;
	Scene scene;
	Input input;
	Input lastInput;
	float dt;
	int frames = 60;
	int outputWidth = 0;
	int outputHeight = 0;
	glm::vec3 min;
	glm::vec3 max;
	float elapsedTime;
	int msaa = 4;

	int selectedModel = 0;

	std::string textureName = "";
	std::string texturePathName = "";
	bool loadingTexture = false;

	bool loadingModel = false;

	std::string meshName = "";
	std::string meshPathName = "";
	bool loadingMesh = false;

	int uiId = 0;
	bool firstTime = true;
};


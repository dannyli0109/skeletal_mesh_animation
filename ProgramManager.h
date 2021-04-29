#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <unordered_map>
#include "Graphics.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "stb_image.h"

#include "Utils.h"


#include "Renderer.h"
#include "ResourceManager.h"
#include "SceneManager.h"

#include "GUI.h"

class ProgramManager
{
public:
	ProgramManager() = default;
	int Init();
	void Update();
	void Destroy();

private:
	GLFWwindow* window;
	ResourceManager resourceManager;
	Scene scene;
};


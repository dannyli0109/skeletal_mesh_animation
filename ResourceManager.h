#pragma once
struct Meshes
{
	Mesh soulSpear;
	Mesh vampire;
	Mesh sphere;
	Mesh quad;
};

struct Skeletons
{
	Bone vampireSkeleton;
};

struct Animations
{
	Animation vampireAnimation;
};

struct ShaderPrograms
{
	ShaderProgram color;
	ShaderProgram phong;
	ShaderProgram output;
};

struct Textures
{
	Texture soulSpearDiffuse;
	Texture soulSpearNormal;
	Texture soulSpearSpecular;

	Texture vampireDiffuse;
	Texture vampireNormal;
	Texture vampireSpecular;
	Texture vampireEmission;
};

struct FrameBuffers
{
	FrameBuffer output;
};



struct ResourceManager
{
	Meshes meshes;
	ShaderPrograms shaders;
	Textures textures;
	Animations animations;
	Skeletons skeletons;
	FrameBuffers frameBuffers;
	LineRenderer lineRenderer;
};

static void OnWindowResize(GLFWwindow* window, int width, int height)
{
	Window* windowData = (Window*)glfwGetWindowUserPointer(window);
	windowData->width = width;
	windowData->height = height;
	windowData->shouldUpdate = true;
}

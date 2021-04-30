#pragma once
#include <vector>

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
};

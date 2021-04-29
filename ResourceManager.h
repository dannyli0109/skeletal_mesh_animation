#pragma once
#include <vector>

struct Meshes
{
	Mesh soulSpear;
	Mesh vampire;
	Mesh sphere;
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
};

struct Textures
{
	Texture soulSpearDiffuse;
	Texture soulSpearNormal;
	Texture soulSpearSpecular;

	Texture vampireDiffuse;
	Texture vampireNormal;
	Texture vampireSpecular;
};

struct ResourceManager
{
	Meshes meshes;
	ShaderPrograms shaders;
	Textures textures;
	Animations animations;
	Skeletons skeletons;
};

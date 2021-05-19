#pragma once

#define COLOR_SHADER 0
#define LINE_SHADER 1
#define PHONG_SHADER 2
#define OUTPUT_SHADER 3
#define NORMAL_SHADER 4
#define UNSHADED_SHADER 5
#define TEXTURE_SHADER 6
#define SPRITE_SHADER 7

#define VAMPIRE_MESH 0
#define SPHERE_MESH 1
#define QUAD_MESH 2

#define VAMPIRE_PHONG_MATERIAL 0

#define VAMPIRE_DIFFUSE 0
#define VAMPIRE_NORMAL 1
#define VAMPIRE_SPECULAR 2
#define VAMPIRE_EMISSION 3
#define WHITE 4

#define MSAA_FRAMEBUFFER 0
#define OUTPUT_FRAMEBUFFER 1
#define SPRITE_FRAMEBUFFER 2

struct Skeletons
{
	Bone vampireSkeleton;
};

struct Animations
{
	Animation vampireAnimation;
};

struct FrameBuffers
{
	FrameBuffer msaa;
	FrameBuffer output;
};

struct SpriteAnimations
{
	std::vector<std::vector<Texture>> textures;
	std::vector<int> widths;
	std::vector<int> heights;
	std::vector<int> currentFrames;
	std::vector<float> durations;
	int count;
};

struct Resource
{
	std::vector<Mesh> meshes;
	std::vector<ShaderProgram> shaders;
	std::vector<Texture> textures;
	std::vector<FrameBuffer> frameBuffers;
	std::vector<Material> materials;
	Animations animations;
	Skeletons skeletons;
	LineRenderer lineRenderer;
	Window window;
	SpriteAnimations spriteAnimations;
	SpriteRenderer sprtieRenderer;
	//Materials materials;
};

static void InitResources(Resource* resource, Window* window)
{
	// 0
	{
		ShaderProgram colorShader = {};
		InitShaderProgram(&colorShader, "Phong.vert", "Color.frag");
		resource->shaders.push_back(colorShader);
		//resource->shaders.color = colorShader;
	}

	// 1
	{
		ShaderProgram lineShader = {};
		InitShaderProgram(&lineShader, "Line.vert", "Line.frag");
		resource->shaders.push_back(lineShader);
		//resource->shaders.line = lineShader;
	}

	// 2
	{
		ShaderProgram phongShader = {};
		InitShaderProgram(&phongShader, "Phong.vert", "Phong.frag");
		resource->shaders.push_back(phongShader);
		//resource->shaders.phong = phongShader;
		SetUniform(&resource->shaders[PHONG_SHADER], "u_diffuseTexture", 0);
		SetUniform(&resource->shaders[PHONG_SHADER], "u_normalTexture", 1);
		SetUniform(&resource->shaders[PHONG_SHADER], "u_specularTexture", 2);
		SetUniform(&resource->shaders[PHONG_SHADER], "u_emissionTexture", 3);
	}

	// 3
	{
		ShaderProgram outputShader = {};
		InitShaderProgram(&outputShader, "Output.vert", "Output.frag");
		resource->shaders.push_back(outputShader);
		SetUniform(&resource->shaders[OUTPUT_SHADER], "u_colourTexture", 0);
	}

	// 4
	{
		ShaderProgram normalShader = {};
		InitShaderProgram(&normalShader, "Phong.vert", "Normal.frag");
		resource->shaders.push_back(normalShader);
		//resource->shaders.phong = phongShader;
		SetUniform(&resource->shaders[NORMAL_SHADER], "u_normalTexture", 0);
	}

	//5 
	{
		ShaderProgram colorShader = {};
		InitShaderProgram(&colorShader, "Color.vert", "Color.frag");
		resource->shaders.push_back(colorShader);
	}

	// 6
	{
		ShaderProgram diffuseShader = {};
		InitShaderProgram(&diffuseShader, "Phong.vert", "Texture.frag");
		resource->shaders.push_back(diffuseShader);
		//resource->shaders.phong = phongShader;
		SetUniform(&resource->shaders[TEXTURE_SHADER], "u_texture", 0);
	}

	// 7 
	{
		ShaderProgram spriteShader = {};
		InitShaderProgram(&spriteShader, "Quad.vert", "Quad.frag");
		resource->shaders.push_back(spriteShader);
	}

	// msaa
	{
		FrameBuffer msaa = {};
		InitFrameBuffer(&msaa, window->width, window->height, 4);
		resource->frameBuffers.push_back(msaa);
	}

	// output
	{
		FrameBuffer output = {};
		InitFrameBuffer(&output, window->width, window->height);
		resource->frameBuffers.push_back(output);
	}

	// sprite
	{
		FrameBuffer spirteFB = {};
		InitFrameBuffer(&spirteFB, window->width, window->height);
		resource->frameBuffers.push_back(spirteFB);
	}


	{
		LineRenderer lineRenderer = {};
		InitLineRenderer(&lineRenderer, 4096);
		resource->lineRenderer = lineRenderer;
	}

	{
		SpriteRenderer spriteRenderer = {};
		spriteRenderer.program = &resource->shaders[SPRITE_SHADER];
		InitSpriteRenderer(&spriteRenderer, 2048);
		resource->sprtieRenderer = spriteRenderer;
	}


	{
		// init vampire
		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile("vampire/dancing_vampire.dae", aiProcess_Triangulate | aiProcess_JoinIdenticalVertices | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);

		Mesh vampireMesh = {};
		Bone vampireSkeleton = {};
		int boneCount = 0;
		MeshData vampireMeshData = LoadMeshData(scene, vampireSkeleton, boneCount);
		std::vector<Animation> animations = LoadAnimations(scene, vampireSkeleton, boneCount);
		resource->skeletons.vampireSkeleton = vampireSkeleton;
		resource->animations.vampireAnimation = animations[0];
		InitMesh("Vampire", &vampireMesh, &vampireMeshData);
		resource->meshes.push_back(vampireMesh);
		resource->animations.vampireAnimation.currentPose.resize(boneCount, glm::mat4(1.0f));

		Texture vampireDiffuseTexture = {};
		LoadTexture(&vampireDiffuseTexture, "Vampire Diffuse", "vampire\\textures\\Vampire_diffuse.png");
		resource->textures.push_back(vampireDiffuseTexture);
		//resource->textures.vampireDiffuse = vampireDiffuseTexture;

		Texture vampireNormalTexture = {};
		LoadTexture(&vampireNormalTexture, "Vampire Normal", "vampire\\textures\\Vampire_normal.png");
		resource->textures.push_back(vampireNormalTexture);
		//resource->textures.vampireNormal = vampireNormalTexture;

		Texture vampireSpecularTexture = {};
		LoadTexture(&vampireSpecularTexture, "Vampire Specular", "vampire\\textures\\Vampire_specular.png");
		resource->textures.push_back(vampireSpecularTexture);
		//resource->textures.vampireSpecular = vampireSpecularTexture;

		Texture vampireEmissionTexture = {};
		LoadTexture(&vampireEmissionTexture, "Vampire Emission", "vampire\\textures\\Vampire_emission.png");
		resource->textures.push_back(vampireEmissionTexture);
		//resource->textures.vampireEmission = vampireEmissionTexture;

	}

	{
		Texture whiteTexture = {};
		LoadTexture(&whiteTexture, "White", "white.png");
		resource->textures.push_back(whiteTexture);
	}

	{
		Material material = {};
		material.shaderProgram = &resource->shaders[PHONG_SHADER];
		material.type = 0;
		material.name = "Phong";
		material.phong.diffuseTexture = &resource->textures[VAMPIRE_DIFFUSE];
		material.phong.normalTexture = &resource->textures[VAMPIRE_NORMAL];
		material.phong.specularTexture = &resource->textures[VAMPIRE_SPECULAR];
		material.phong.emissionTexture = &resource->textures[VAMPIRE_EMISSION];

		material.phong.ka = { 1.0f, 1.0f, 1.0f };
		material.phong.kd = { 1.0f, 1.0f, 1.0f };
		material.phong.ks = { 1.0f, 1.0f, 1.0f };
		material.phong.ke = { 1.0f, 1.0f, 1.0f };
		material.phong.specularPower = 8.0f;

		resource->materials.push_back(material);
	}

	{
		Material material = {};
		material.shaderProgram = &resource->shaders[COLOR_SHADER];
		material.type = 1;
		material.name = "Color";
		material.color.color = { 1.0f, 1.0f, 1.0f };

		resource->materials.push_back(material);
	}

	{
		Material material = {};
		material.shaderProgram = &resource->shaders[NORMAL_SHADER];
		material.type = 2;
		material.name = "Normal";
		material.normal.texture = &resource->textures[VAMPIRE_NORMAL];

		resource->materials.push_back(material);
	}

	{
		Material material = {};
		material.shaderProgram = &resource->shaders[TEXTURE_SHADER];
		material.type = 3;
		material.name = "Diffuse";
		material.diffuse.texture = &resource->textures[VAMPIRE_DIFFUSE];
		resource->materials.push_back(material);
	}

	{
		Material material = {};
		material.shaderProgram = &resource->shaders[TEXTURE_SHADER];
		material.type = 4;
		material.name = "Specular";
		material.specular.texture = &resource->textures[VAMPIRE_SPECULAR];
		resource->materials.push_back(material);
	}

	{
		Material material = {};
		material.shaderProgram = &resource->shaders[TEXTURE_SHADER];
		material.type = 5;
		material.name = "Emission";
		material.diffuse.texture = &resource->textures[VAMPIRE_EMISSION];
		resource->materials.push_back(material);
	}

	{
		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile("sphere.obj", aiProcess_Triangulate | aiProcess_JoinIdenticalVertices | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);
		Mesh sphereMesh = {};
		MeshData sphereMeshData = LoadMeshData(scene);
		InitMesh("Sphere", &sphereMesh, &sphereMeshData);
		resource->meshes.push_back(sphereMesh);
	}

	// quad mesh
	{
		MeshData quadMeshData = {};
		VertexData v0 = {};
		v0.mesh.position = { -0.5f, 0.5f, 0.5f };
		v0.mesh.uv = { 0, 0 };

		VertexData v1 = {};
		v1.mesh.position = { 0.5f, 0.5f, 0.5f };
		v1.mesh.uv = { 1.0f, 0 };

		VertexData v2 = {};
		v2.mesh.position = { -0.5f, -0.5f, 0.5f };
		v2.mesh.uv = { 0, 1.0f };

		VertexData v3 = {};
		v3.mesh.position = { 0.5f, -0.5f, 0.5f };
		v3.mesh.uv = { 1.0f, 1.0f };

		quadMeshData.vertices.resize(4);
		quadMeshData.vertices[0] = v0;
		quadMeshData.vertices[1] = v1;
		quadMeshData.vertices[2] = v2;
		quadMeshData.vertices[3] = v3;
		quadMeshData.indices = { 0, 1, 2, 1, 3, 2 };

		Mesh quadMesh = {};
		InitMesh("Quad", &quadMesh, &quadMeshData);
		resource->meshes.push_back(quadMesh);
	}
	resource->spriteAnimations = {};
}

static void AddSpriteAnimation(SpriteAnimations* spriteAnimations, std::vector<Texture> textures, int width, int height, float duration)
{
	spriteAnimations->textures.push_back(textures);
	spriteAnimations->widths.push_back(width);
	spriteAnimations->heights.push_back(height);
	spriteAnimations->currentFrames.push_back(0);
	spriteAnimations->durations.push_back(duration);
	spriteAnimations->count++;
}

static void OnWindowResize(GLFWwindow* window, int width, int height)
{
	Window* windowData = (Window*)glfwGetWindowUserPointer(window);
	windowData->width = width;
	windowData->height = height;
	windowData->shouldUpdate = true;
}

static void DestroyResources(Resource* resource, Window* window)
{
	for (int i = 0; i < resource->textures.size(); i++)
	{
		glDeleteTextures(1, &resource->textures[i].id);
	}

	for (int i = 0; i < resource->spriteAnimations.count; i++)
	{
		for (int j = 0; j < resource->spriteAnimations.textures[i].size(); j++)
		{
			glDeleteTextures(1, &resource->spriteAnimations.textures[i][j].id);
		}
	}

	for (int i = 0; i < resource->frameBuffers.size(); i++)
	{
		glDeleteFramebuffers(1, &resource->frameBuffers[i].fbo);
		glDeleteRenderbuffers(1, &resource->frameBuffers[i].rbo);
		glDeleteTextures(1, &resource->frameBuffers[i].texture.id);
	}

	for (int i = 0; i < resource->meshes.size(); i++)
	{
		glDeleteVertexArrays(1, &resource->meshes[i].vao);
		glDeleteBuffers(1, &resource->meshes[i].vertexBuffer);
		glDeleteBuffers(1, &resource->meshes[i].indexBuffer);
	}

	glDeleteVertexArrays(1, &resource->lineRenderer.vao);
	glDeleteVertexArrays(1, &resource->lineRenderer.vertexBuffer);
}

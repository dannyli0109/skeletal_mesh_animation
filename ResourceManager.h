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
	ShaderProgram line;
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

struct Materials
{
	Material vampirePhongMaterial;
};


struct Resource
{
	Meshes meshes;
	ShaderPrograms shaders;
	Textures textures;
	Animations animations;
	Skeletons skeletons;
	FrameBuffers frameBuffers;
	Materials materials;
	LineRenderer lineRenderer;
	Window window;
};

static void InitResources(Resource* resource, Window* window)
{
	{
		ShaderProgram colorShader = {};
		InitShaderProgram(&colorShader, "Color.vert", "Color.frag");
		resource->shaders.color = colorShader;
	}

	{
		ShaderProgram lineShader = {};
		InitShaderProgram(&lineShader, "Line.vert", "Line.frag");
		resource->shaders.line = lineShader;
	}

	{
		ShaderProgram phongShader = {};
		InitShaderProgram(&phongShader, "Phong.vert", "Phong.frag");
		resource->shaders.phong = phongShader;
		SetUniform(&resource->shaders.phong, "u_diffuseTexture", 0);
		SetUniform(&resource->shaders.phong, "u_normalTexture", 1);
		SetUniform(&resource->shaders.phong, "u_specularTexture", 2);
		SetUniform(&resource->shaders.phong, "u_emissionTexture", 3);
	}

	{
		ShaderProgram outputShader = {};
		InitShaderProgram(&outputShader, "Output.vert", "Output.frag");
		resource->shaders.output = outputShader;
		SetUniform(&resource->shaders.output, "u_colourTexture", 0);
	}

	{
        FrameBuffer fb = {};
        InitFrameBuffer(&fb, window->width, window->height);
        resource->frameBuffers.output = fb;
    }

	{
		LineRenderer lineRenderer = {};
		InitLineRenderer(&lineRenderer, 4096);
		resource->lineRenderer = lineRenderer;
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
		InitMesh(&vampireMesh, &vampireMeshData);
		resource->meshes.vampire = vampireMesh;
		Texture vampireDiffuseTexture = {};
		LoadTexture(&vampireDiffuseTexture, "vampire\\textures\\Vampire_diffuse.png");
		resource->textures.vampireDiffuse = vampireDiffuseTexture;

		Texture vampireNormalTexture = {};
		LoadTexture(&vampireNormalTexture, "vampire\\textures\\Vampire_normal.png");
		resource->textures.vampireNormal = vampireNormalTexture;

		Texture vampireSpecularTexture = {};
		LoadTexture(&vampireSpecularTexture, "vampire\\textures\\Vampire_specular.png");
		resource->textures.vampireSpecular = vampireSpecularTexture;

		Texture vampireEmissionTexture = {};
		LoadTexture(&vampireEmissionTexture, "vampire\\textures\\Vampire_emission.png");
		resource->textures.vampireEmission = vampireEmissionTexture;

		resource->animations.vampireAnimation.currentPose.resize(boneCount, glm::mat4(1.0f));
	}

	{
		Material material = {};
		material.phong.diffuseTexture = resource->textures.vampireDiffuse;
		material.phong.normalTexture = resource->textures.vampireNormal;
		material.phong.specularTexture = resource->textures.vampireSpecular;
		material.phong.emissionTexture = resource->textures.vampireEmission;

		material.phong.ka = { 1.0f, 1.0f, 1.0f };
		material.phong.kd = { 1.0f, 1.0f, 1.0f };
		material.phong.ks = { 1.0f, 1.0f, 1.0f };
		material.phong.ke = { 1.0f, 1.0f, 1.0f };
		material.phong.specularPower = 8.0f;

		resource->materials.vampirePhongMaterial = material;
	}

	{
		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile("sphere.obj", aiProcess_Triangulate | aiProcess_JoinIdenticalVertices | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);
		Mesh sphereMesh = {};
		MeshData sphereMeshData = LoadMeshData(scene);
		InitMesh(&sphereMesh, &sphereMeshData);
		resource->meshes.sphere = sphereMesh;
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
		InitMesh(&quadMesh, &quadMeshData);
		resource->meshes.quad = quadMesh;
	}
}

static void OnWindowResize(GLFWwindow* window, int width, int height)
{
	Window* windowData = (Window*)glfwGetWindowUserPointer(window);
	windowData->width = width;
	windowData->height = height;
	windowData->shouldUpdate = true;
}

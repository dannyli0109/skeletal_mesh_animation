#define MAX_BONE_INFLUENCE 4


struct ShaderProgram
{
	GLuint vertexShader;
	GLuint fragmentShader;
	GLuint shaderProgram;
};

struct Texture
{
	GLuint id;
	std::string name;
	int width;
	int height;
};

struct VertexData
{
	union
	{
		struct {
			glm::vec3 position;
			glm::vec3 normal;
			glm::vec3 vertTangent;
			glm::vec3 vertBitangent;
			glm::vec3 color;
			glm::vec2 uv;
		} mesh;

		struct
		{
			glm::vec3 position;
			glm::vec4 color;
		} line;
	};

	struct {
		int boneIDs[MAX_BONE_INFLUENCE];
		float weights[MAX_BONE_INFLUENCE];
	} animated;
};


struct Mesh
{
	std::string name;
	GLuint vertexBuffer;
	GLuint indexBuffer;
	GLuint vao;
	std::vector<VertexData> vertices;
	unsigned int indexCount;
	bool initialised;
};

struct Bone
{
	int id;
	std::string name;
	glm::mat4 offset;
	std::vector<Bone> children;
};

struct Skeleton
{
	Bone root;
	int count;
};


struct MeshData
{
	std::vector<VertexData> vertices;
	std::vector<unsigned short> indices;
};


struct BoneTransformTrack
{
	std::vector<float> positionTimestamps;
	std::vector<float> rotationTimestamps;
	std::vector<float> scaleTimestamps;

	std::vector<glm::vec3> positions;
	std::vector<glm::quat> rotations;
	std::vector<glm::vec3> scales;
};

struct Animation
{
	std::string name;
	float duration;
	int ticksPersecond;
	std::unordered_map<std::string, BoneTransformTrack> boneTransforms;
	glm::mat4 globalInverseTransform;
	std::vector<glm::mat4> currentPose;
	Bone skeleton;
	int boneCount;
};

struct Camera
{
	glm::vec3 position;
	glm::vec3 up;
	float theta;
	float phi;

	float fovY;
	float aspect;
	float near;
	float far;
};

struct Camera2D
{
	glm::vec2 position;
	glm::vec2 windowSize;
	float zoom;
	float zoomSpeed;
	float aspect;
};

struct FrameBuffer
{
	GLuint fbo;
	GLuint rbo;
	Texture texture;
	int width;
	int height;
	bool initialised;
};

struct LineRenderer
{
	GLuint vertexBuffer;
	GLuint vao;
	std::vector<VertexData> vertices;
	int maxSize;
};

struct SpriteVertex
{
	glm::vec3 position;
	glm::vec2 uv;
	glm::vec4 color;
	float textureIndex;
	glm::vec2 tiling;
};

struct SpriteRenderer
{
	ShaderProgram* program;
	int maxIndices;
	int maxVertices;
	int indexCount;
	int vertexCount;
	std::vector<SpriteVertex> vertices;
	std::vector<Texture*> textureSlots;
	int maxTextureSlot;
	int textureSlotIndex;
	GLuint vertexBuffer;
	GLuint indexBuffer;
	GLuint vao;
};

static glm::vec4 quadPositions[4] = {
	{-0.5f, 0.5f, 0, 1.0f},
	{0.5f, 0.5f, 0, 1.0f},
	{-0.5f, -0.5f, 0, 1.0f},
	{0.5f, -0.5f, 0, 1.0f}
};

static glm::vec2 quadUvs[4] = {
	{0, 0},
	{1, 0},
	{0, 1},
	{1, 1}
};

struct Material
{
	std::string name;
	int type;
	ShaderProgram* shaderProgram;
	union
	{
		struct {
			Texture* diffuseTexture;
			Texture* normalTexture;
			Texture* specularTexture;
			Texture* emissionTexture;

			glm::vec3 ka;
			glm::vec3 kd;
			glm::vec3 ks;
			glm::vec3 ke;
			float specularPower;
		} phong;

		struct {
			glm::vec3 color;
		} color;

		struct {
			Texture* texture;
		} normal;

		struct {
			Texture* texture;
		} diffuse;

		struct {
			Texture* texture;
		} specular;

		struct {
			Texture* texture;
		} emission;
	};
};


// Shader program
static void InitShaderProgram(ShaderProgram* program, std::string vertexFileName, std::string fragmentFileName)
{
    // Init
    program->vertexShader = glCreateShader(GL_VERTEX_SHADER);
    program->fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    program->shaderProgram = glCreateProgram();
    bool loaded = true;

    // Vertex Shader
    std::string vertexSource = LoadFileAsString(vertexFileName);
    const char* vertexSourceC = vertexSource.c_str();
    glShaderSource(program->vertexShader, 1, &vertexSourceC, nullptr);
    glCompileShader(program->vertexShader);

    // Log error for vertex shader
    GLchar errorLog[512];
    GLint success = 0;
    glGetShaderiv(program->vertexShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        std::cout << "Vertex shader " << vertexFileName << " failed with error:" << std::endl;
        glGetShaderInfoLog(program->vertexShader, 512, nullptr, errorLog);
        std::cout << errorLog << std::endl;
        loaded = false;
    }
    else
    {
        std::cout << vertexFileName << " compiled successfully." << std::endl;
    }

    // Fragment Shader
    std::string fragmentSource = LoadFileAsString(fragmentFileName);
    const char* fragmentSourceC = fragmentSource.c_str();
    glShaderSource(program->fragmentShader, 1, &fragmentSourceC, nullptr);
    glCompileShader(program->fragmentShader);

    glGetShaderiv(program->fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        std::cout << "Fragment shader " << fragmentFileName << " failed with error:" << std::endl;
        glGetShaderInfoLog(program->fragmentShader, 512, nullptr, errorLog);
        std::cout << errorLog << std::endl;
        loaded = false;
    }
    else
    {
        std::cout << fragmentFileName << " compiled successfully." << std::endl;
    }

    // Attach shaders to the shader program
    glAttachShader(program->shaderProgram, program->vertexShader);
    glAttachShader(program->shaderProgram, program->fragmentShader);
    glLinkProgram(program->shaderProgram);
    glGetProgramiv(program->shaderProgram, GL_LINK_STATUS, &success);

    if (!success)
    {
        std::cout << "Error linking shaders " << vertexFileName << " and " << fragmentFileName << std::endl;
        glGetProgramInfoLog(program->shaderProgram, 512, nullptr, errorLog);
        std::cout << errorLog << std::endl;
        loaded = false;
    }

    if (loaded)
    {
        std::cout << "Shader program linked successfully" << std::endl;
    }
}

static void SetUniform(ShaderProgram* program, std::string varname, float value)
{
	GLuint varloc = glGetUniformLocation(program->shaderProgram, varname.c_str());
	glUseProgram(program->shaderProgram);
	glUniform1f(varloc, value);
}

static void SetUniform(ShaderProgram* program, std::string varname, float& value, int count)
{
	GLuint varloc = glGetUniformLocation(program->shaderProgram, varname.c_str());
	glUseProgram(program->shaderProgram);
	glUniform1fv(varloc, count, &value);
}

static void SetUniform(ShaderProgram* program, std::string varname, glm::mat4 value)
{
	GLuint varloc = glGetUniformLocation(program->shaderProgram, varname.c_str());
	glUseProgram(program->shaderProgram);
	glUniformMatrix4fv(varloc, 1, GL_FALSE, &value[0][0]);
}

static void SetUniform(ShaderProgram* program, std::string varname, glm::mat4& value, int count)
{
	GLuint varloc = glGetUniformLocation(program->shaderProgram, varname.c_str());
	glUseProgram(program->shaderProgram);
	glUniformMatrix4fv(varloc, count, GL_FALSE, &value[0][0]);
}

static void SetUniform(ShaderProgram* program, std::string varname, glm::vec3 value)
{
	GLuint varloc = glGetUniformLocation(program->shaderProgram, varname.c_str());
	glUseProgram(program->shaderProgram);
	glUniform3fv(varloc, 1, &value[0]);
}

static void SetUniform(ShaderProgram* program, std::string varname, glm::vec3& value, int count)
{
	GLuint varloc = glGetUniformLocation(program->shaderProgram, varname.c_str());
	glUseProgram(program->shaderProgram);
	glUniform3fv(varloc, count, &value[0]);
}

static void SetUniform(ShaderProgram* program, std::string varname, int value)
{
	GLuint varloc = glGetUniformLocation(program->shaderProgram, varname.c_str());
	glUseProgram(program->shaderProgram);
	glUniform1i(varloc, value);
}


// Texture
static bool LoadTexture(Texture* texture, std::string name, std::string filename)
{
	bool loaded = false;
	int width, height, channels;
	unsigned char* data = stbi_load(filename.c_str(), &width, &height, &channels, 0);
	glGenTextures(1, &texture->id);
	glBindTexture(GL_TEXTURE_2D, texture->id);
	texture->width = width;
	texture->height = height;
	if (data)
	{
		texture->name = name;
		if (channels == 3)
		{
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
			glTextureParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTextureParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glGenerateMipmap(GL_TEXTURE_2D);
			loaded = true;
		}
		else if (channels == 4)
		{
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
			glTextureParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTextureParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glGenerateMipmap(GL_TEXTURE_2D);
			loaded = true;
		}
		else {
			std::cout << "unknown texture type" << std::endl;
			loaded = false;
		}
	}
	else {
		std::cout << "Fail to load texture" << std::endl;
		loaded = false;
	}

	glBindTexture(GL_TEXTURE_2D, 0);
	stbi_image_free(data);
	if (!loaded)
	{
		glDeleteTextures(1, &texture->id);
		return false;
	}
	return true;
}

static void InitTexture(Texture* texture, char* data, int width, int height)
{
	texture->width = width;
	texture->height = height;
	glGenTextures(1, &texture->id);
	glBindTexture(GL_TEXTURE_2D, texture->id);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glBindTexture(GL_TEXTURE_2D, 0);
}


static void BindTexture(Texture* texture, int textureUnit)
{
	glActiveTexture(GL_TEXTURE0 + textureUnit);
	glBindTexture(GL_TEXTURE_2D, texture->id);
}

static void UnbindTexture()
{
	glBindTexture(GL_TEXTURE_2D, 0);
}

static glm::mat4 ConvertAssimpToGLM(aiMatrix4x4 matrix)
{
	glm::mat4 result;
	result[0][0] = matrix.a1;
	result[1][0] = matrix.a2;
	result[2][0] = matrix.a3;
	result[3][0] = matrix.a4;

	result[0][1] = matrix.b1;
	result[1][1] = matrix.b2;
	result[2][1] = matrix.b3;
	result[3][1] = matrix.b4;	
	
	result[0][2] = matrix.c1;
	result[1][2] = matrix.c2;
	result[2][2] = matrix.c3;
	result[3][2] = matrix.c4;

	result[0][3] = matrix.d1;
	result[1][3] = matrix.d2;
	result[2][3] = matrix.d3;
	result[3][3] = matrix.d4;
	
	return result;
}

static glm::vec3 ConvertAssimpToGLM(aiVector3D vec)
{
	return glm::vec3(vec.x, vec.y, vec.z);
}

static glm::quat ConvertAssimpToGLM(aiQuaternion quat)
{
	glm::quat q;
	q.x = quat.x;
	q.y = quat.y;
	q.z = quat.z;
	q.w = quat.w;
	return q;
}

// Mesh

static bool ReadBone(Bone& bone, aiNode* node, std::unordered_map<std::string, std::pair<int, glm::mat4>>& boneInfoTable)
{
	if (boneInfoTable.find(node->mName.C_Str()) != boneInfoTable.end())
	{
		bone.name = node->mName.C_Str();
		bone.id = boneInfoTable[bone.name].first;
		bone.offset = boneInfoTable[bone.name].second;

		for (int i = 0; i < node->mNumChildren; i++)
		{
			Bone child;
			ReadBone(child, node->mChildren[i], boneInfoTable);
			bone.children.push_back(child);
		}
		return true;
	}
	else
	{
		for (int i = 0; i < node->mNumChildren; i++)
		{
			if (ReadBone(bone, node->mChildren[i], boneInfoTable))
			{
				return true;
			}
		}
	}
	return false;
}

static MeshData LoadMeshData(const aiScene* scene, int index)
{
	MeshData meshData = {};

	aiMesh* meshInfo = scene->mMeshes[index];
	meshData.vertices.reserve(meshInfo->mNumVertices);
	meshData.indices.reserve(meshInfo->mNumFaces);

	for (int i = 0; i < meshInfo->mNumVertices; i++)
	{
		VertexData newVertex = {};
		newVertex.mesh.position = { meshInfo->mVertices[i].x, meshInfo->mVertices[i].y, meshInfo->mVertices[i].z };

		if (meshInfo->HasVertexColors(0))
		{
			newVertex.mesh.color = { meshInfo->mColors[0][i].r, meshInfo->mColors[0][i].g, meshInfo->mColors[0][i].b };
		}
		else
		{
			newVertex.mesh.color = { 1, 1, 1 };
		}

		if (meshInfo->HasTextureCoords(0))
		{
			newVertex.mesh.uv = { meshInfo->mTextureCoords[0][i].x, meshInfo->mTextureCoords[0][i].y };
		}
		else
		{
			newVertex.mesh.uv = { newVertex.mesh.position.x, newVertex.mesh.position.y };
		}

		if (meshInfo->HasNormals())
		{
			newVertex.mesh.normal = { meshInfo->mNormals[i].x, meshInfo->mNormals[i].y, meshInfo->mNormals[i].z };
		}
		else
		{
			newVertex.mesh.normal = { 0, 0, 0 };
		}

		if (meshInfo->HasTangentsAndBitangents())
		{
			newVertex.mesh.vertTangent = { meshInfo->mTangents[i].x, meshInfo->mTangents[i].y, meshInfo->mTangents[i].z };
			newVertex.mesh.vertBitangent = { meshInfo->mBitangents[i].x, meshInfo->mBitangents[i].y, meshInfo->mBitangents[i].z };
		}
		else
		{
			newVertex.mesh.vertTangent = { 0, 0, 0 };
			newVertex.mesh.vertBitangent = { 0, 0, 0 };
		}
		
		meshData.vertices.push_back(newVertex);
	}


	for (int i = 0; i < meshInfo->mNumFaces; i++)
	{
		meshData.indices.push_back(meshInfo->mFaces[i].mIndices[0]);
		meshData.indices.push_back(meshInfo->mFaces[i].mIndices[1]);
		meshData.indices.push_back(meshInfo->mFaces[i].mIndices[2]);
	}

	
	return meshData;

}

static void LoadBoneData(const aiScene* scene, MeshData* meshData, Bone& skeleton, int& boneCount)
{
	//MeshData meshData = LoadMeshData(scene);
	aiMesh* meshInfo = scene->mMeshes[0];

	std::unordered_map<std::string, std::pair<int, glm::mat4>> boneInfo = {};
	std::vector<int> boneCounts;
	boneCounts.resize(meshData->vertices.size(), 0);
	for (int i = 0; i < meshInfo->mNumBones; i++)
	{
		aiBone* bone = meshInfo->mBones[i];
		glm::mat4 m = ConvertAssimpToGLM(bone->mOffsetMatrix);
		boneInfo[bone->mName.C_Str()] = { i, m };

		for (int j = 0; j < bone->mNumWeights; j++)
		{
			int id = bone->mWeights[j].mVertexId;
			float weight = bone->mWeights[j].mWeight;

			switch (boneCounts[id])
			{
			case 0:
				meshData->vertices[id].animated.boneIDs[0] = i;
				meshData->vertices[id].animated.weights[0] = weight;
				break;
			case 1:
				meshData->vertices[id].animated.boneIDs[1] = i;
				meshData->vertices[id].animated.weights[1] = weight;
				break;
			case 2:
				meshData->vertices[id].animated.boneIDs[2] = i;
				meshData->vertices[id].animated.weights[2] = weight;
				break;
			case 3:
				meshData->vertices[id].animated.boneIDs[3] = i;
				meshData->vertices[id].animated.weights[3] = weight;
				break;
			default:
				break;
			}
			boneCounts[id]++;
		}
	}

	boneCount = meshInfo->mNumBones;
	ReadBone(skeleton, scene->mRootNode, boneInfo);
}

static std::vector<Animation> LoadAnimations(const aiScene* scene, MeshData* meshData)
{
	std::vector<Animation> animations;
	aiAnimation** animationInfos = scene->mAnimations;
	aiNode* root = scene->mRootNode;

	Bone skeleton = {};
	int boneCount = 0;

	LoadBoneData(scene, meshData, skeleton, boneCount);

	for (int i = 0; i < scene->mNumAnimations; i++)
	{
		aiAnimation* anim = animationInfos[i];
		Animation animation = {};
		animation.duration = anim->mDuration;
		animation.ticksPersecond = anim->mTicksPerSecond;
		animation.globalInverseTransform = glm::inverse(ConvertAssimpToGLM(scene->mRootNode->mTransformation));
		animation.boneCount = boneCount;
		animation.skeleton = skeleton;
		animation.name = anim->mName.C_Str();

		for (int j = 0; j < anim->mNumChannels; j++)
		{
			aiNodeAnim* channel = anim->mChannels[j];
			BoneTransformTrack track;
			for (int k = 0; k < channel->mNumPositionKeys; k++)
			{
				track.positionTimestamps.push_back(channel->mPositionKeys[k].mTime);
				track.positions.push_back(ConvertAssimpToGLM(channel->mPositionKeys[k].mValue));
			}

			for (int k = 0; k < channel->mNumRotationKeys; k++)
			{
				track.rotationTimestamps.push_back(channel->mRotationKeys[k].mTime);
				track.rotations.push_back(ConvertAssimpToGLM(channel->mRotationKeys[k].mValue));
			}

			for (int k = 0; k < channel->mNumScalingKeys; k++)
			{
				track.scaleTimestamps.push_back(channel->mScalingKeys[k].mTime);
				track.scales.push_back(ConvertAssimpToGLM(channel->mScalingKeys[k].mValue));
			}
			animation.boneTransforms[channel->mNodeName.C_Str()] = track;
		}
		animations.push_back(animation);
	}
	return animations;
}

static std::pair<int, float> GetTimeFraction(std::vector<float>& times, float& dt) {
	int segment = 0;
	while (dt >= times[segment])
		segment++;
	float start = times[segment - 1];
	float end = times[segment];
	float frac = (dt - start) / (end - start);
	return { segment, frac };
}


static void GetPose(Animation* animation, Bone& skeleton, float dt, glm::mat4& parentTransform)
{
	if (animation->boneTransforms.find(skeleton.name) == animation->boneTransforms.end()) return;
	BoneTransformTrack& btt = animation->boneTransforms[skeleton.name];
	dt = fmod(dt, animation->duration);
	std::pair<int, float> fp;
	fp = GetTimeFraction(btt.positionTimestamps, dt);

	glm::vec3 position1 = btt.positions[fp.first - 1];
	glm::vec3 position2 = btt.positions[fp.first];

	glm::vec3 position = glm::mix(position1, position2, fp.second);

	fp = GetTimeFraction(btt.rotationTimestamps, dt);
	glm::quat rotation1 = btt.rotations[fp.first - 1];
	glm::quat rotation2 = btt.rotations[fp.first];

	glm::quat rotation = glm::slerp(rotation1, rotation2, fp.second);

	fp = GetTimeFraction(btt.scaleTimestamps, dt);
	glm::vec3 scale1 = btt.scales[fp.first - 1];
	glm::vec3 scale2 = btt.scales[fp.first];

	glm::vec3 scale = glm::mix(scale1, scale2, fp.second);

	glm::mat4 positionMat = glm::mat4(1.0f);
	glm::mat4 scaleMat = glm::mat4(1.0f);

	positionMat = glm::translate(positionMat, position);
	glm::mat4 rotationMat = glm::toMat4(rotation);
	scaleMat = glm::scale(scaleMat, scale);
	glm::mat4 localTransform = positionMat * rotationMat * scaleMat;
	glm::mat4 globalTransform = parentTransform * localTransform;

	/*output[skeleton.id] = animation.globalInverseTransform * globalTransform * skeleton.offset;
	outputBone[skeleton.id] = globalTransform;*/
	animation->currentPose[skeleton.id] = animation->globalInverseTransform * globalTransform * skeleton.offset;

	for (Bone& child : skeleton.children)
	{
		GetPose(animation, child, dt, globalTransform);
	}
}

static void InitMesh(std::string name, Mesh* mesh, MeshData* meshData)
{
	unsigned int indexCount = meshData->indices.size();
	unsigned int vertexCount = meshData->vertices.size();
	VertexData* vertices = meshData->vertices.data();
	unsigned short* indices = meshData->indices.data();

	*mesh = {};
	mesh->indexCount = indexCount;
	mesh->vertices = meshData->vertices;
	mesh->name = name;

	// generate buffers
	glGenBuffers(1, &mesh->vertexBuffer);
	glGenVertexArrays(1, &mesh->vao);

	// bind vertex array
	glBindBuffer(GL_ARRAY_BUFFER, mesh->vertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, vertexCount * sizeof(VertexData), vertices, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);


	glGenBuffers(1, &mesh->indexBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->indexBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexCount * sizeof(unsigned short), indices, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	glBindVertexArray(mesh->vao);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->indexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, mesh->vertexBuffer);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexData), (const void*)offsetof(VertexData, VertexData::mesh.position));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(VertexData), (const void*)offsetof(VertexData, VertexData::mesh.normal));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(VertexData), (const void*)offsetof(VertexData, VertexData::mesh.vertTangent));
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(VertexData), (const void*)offsetof(VertexData, VertexData::mesh.vertBitangent));
	glEnableVertexAttribArray(4);
	glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(VertexData), (const void*)offsetof(VertexData, VertexData::mesh.color));
	glEnableVertexAttribArray(5);
	glVertexAttribPointer(5, 2, GL_FLOAT, GL_FALSE, sizeof(VertexData), (const void*)offsetof(VertexData, VertexData::mesh.uv));
	
	glEnableVertexAttribArray(6);
	glVertexAttribPointer(6, 4, GL_FLOAT, GL_INT, sizeof(VertexData), (const void*)offsetof(VertexData, VertexData::animated.boneIDs));
	glEnableVertexAttribArray(7);
	glVertexAttribPointer(7, 4, GL_FLOAT, GL_FALSE, sizeof(VertexData), (const void*)offsetof(VertexData, VertexData::animated.weights));
	
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

static void DrawMesh(Mesh* mesh)
{
	glBindVertexArray(mesh->vao);
	glDrawElements(GL_TRIANGLES, mesh->indexCount, GL_UNSIGNED_SHORT, 0);
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

static void InitLineRenderer(LineRenderer* lineRenderer, int maxSize) {
	lineRenderer->maxSize = maxSize;
	glGenBuffers(1, &lineRenderer->vertexBuffer);
	glGenVertexArrays(1, &lineRenderer->vao);

	glBindVertexArray(lineRenderer->vao);
	glBindBuffer(GL_ARRAY_BUFFER, lineRenderer->vertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, lineRenderer->maxSize * sizeof(VertexData), nullptr, GL_DYNAMIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexData), (const void*)offsetof(VertexData, VertexData::line.position));
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(VertexData), (const void*)offsetof(VertexData, VertexData::line.color));

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	// Unbind
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

static void ClearRenderer(LineRenderer* lineRenderer)
{
	lineRenderer->vertices.clear();
}

static void Render(LineRenderer* lineRenderer) {
	if (lineRenderer->vertices.size() == 0) return;
	glBindBuffer(GL_ARRAY_BUFFER, lineRenderer->vertexBuffer);
	glBufferSubData(GL_ARRAY_BUFFER, 0, lineRenderer->vertices.size() * sizeof(VertexData), lineRenderer->vertices.data());
	glBindVertexArray(lineRenderer->vao);
	glDrawArrays(GL_LINES, 0, lineRenderer->vertices.size());
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

static void AddLine(LineRenderer* lineRenderer, glm::vec3 p1, glm::vec3 p2, glm::vec4 color)
{
	if (lineRenderer->vertices.size() >= lineRenderer->maxSize)
	{
		Render(lineRenderer);
		ClearRenderer(lineRenderer);
	}
	VertexData v1 = {};
	v1.line.position = p1;
	v1.line.color = color;

	VertexData v2 = {};
	v2.line.position = p2;
	v2.line.color = color;

	lineRenderer->vertices.push_back(v1);
	lineRenderer->vertices.push_back(v2);
}

// Camera
static glm::mat4 GetProjectionMatrix(Camera* camera)
{
	return glm::perspective(camera->fovY, camera->aspect, camera->near, camera->far);
}
static glm::mat4 GetViewMatrix(Camera* camera)
{
	glm::vec3 forward(std::cos(camera->phi) * std::cos(camera->theta), std::sin(camera->phi), std::cos(camera->phi) * std::sin(camera->theta));
	return glm::lookAt(camera->position, camera->position + forward, camera->up);
}
static void HandleCameraController(Camera* camera, Input* input, Input* lastInput, GLFWwindow* window, float dt, float moveSpeed, float turnSpeed)
{
	
	glm::vec3 forward(cosf(camera->phi) * cosf(camera->theta), sinf(camera->phi), sinf(camera->theta));
	glm::vec3 right(-sinf(camera->theta), 0, cos(camera->theta));
	glm::vec3 up(0, 1, 0);

	if (input->wKey)
		camera->position += forward * moveSpeed * dt;
	if (input->sKey)
		camera->position -= forward * moveSpeed * dt;
	if (input->aKey)
		camera->position -= right * moveSpeed * dt;
	if (input->dKey)
		camera->position += right * moveSpeed * dt;

	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
	{
		camera->theta += turnSpeed * (input->mouseX - lastInput->mouseX) * dt;
		camera->phi -= turnSpeed * (input->mouseY - lastInput->mouseY) * dt;
	}
}

static void HandleCameraController2D(Camera2D* camera2D, Input* input, GLFWwindow* window, float dt, float moveSpeed)
{

}


// Frame Buffer ms
static void InitFrameBuffer(FrameBuffer* frameBuffer, int width, int height, int samples)
{
	if (frameBuffer->initialised)
	{
		glDeleteFramebuffers(1, &frameBuffer->fbo);
		glDeleteRenderbuffers(1, &frameBuffer->rbo);
		glDeleteTextures(1, &frameBuffer->texture.id);
		frameBuffer->initialised = false;
	}

	glGenFramebuffers(1, &frameBuffer->fbo);
	glGenTextures(1, &frameBuffer->texture.id);
	glGenRenderbuffers(1, &frameBuffer->rbo);
	frameBuffer->width = width;
	frameBuffer->height = height;

	// glBindTexture(GL_TEXTURE_2D, frameBuffer->texture.id);
	// glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
	
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, frameBuffer->texture.id);
	glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, GL_RGBA, width, height, GL_TRUE);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer->fbo);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, frameBuffer->texture.id, 0);
	// glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, frameBuffer->texture.id, 0);

	glBindRenderbuffer(GL_RENDERBUFFER, frameBuffer->rbo);
	//glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
	glRenderbufferStorageMultisample(GL_RENDERBUFFER, samples, GL_DEPTH32F_STENCIL8, width, height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, frameBuffer->rbo);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE)
	{
		std::cout << "Frame buffer initialised successfully" << std::endl;
		frameBuffer->initialised = true;
	}
	else {
		std::cout << "Failed to initialise frame buffer" << std::endl;
		glDeleteFramebuffers(1, &frameBuffer->fbo);
		glDeleteRenderbuffers(1, &frameBuffer->rbo);
		glDeleteTextures(1, &frameBuffer->texture.id);
		frameBuffer->initialised = false;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
}

// Frame Buffer ms
static void InitFrameBuffer(FrameBuffer* frameBuffer, int width, int height)
{
	if (frameBuffer->initialised)
	{
		glDeleteFramebuffers(1, &frameBuffer->fbo);
		//glDeleteRenderbuffers(1, &frameBuffer->rbo);
		glDeleteTextures(1, &frameBuffer->texture.id);
		frameBuffer->initialised = false;
	}

	glGenFramebuffers(1, &frameBuffer->fbo);
	glGenTextures(1, &frameBuffer->texture.id);
	//glGenRenderbuffers(1, &frameBuffer->rbo);
	frameBuffer->width = width;
	frameBuffer->height = height;

	 glBindTexture(GL_TEXTURE_2D, frameBuffer->texture.id);
	 glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer->fbo);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, frameBuffer->texture.id, 0);

	//glBindRenderbuffer(GL_RENDERBUFFER, frameBuffer->rbo);
	//glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH32F_STENCIL8, width, height);
	//glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, frameBuffer->rbo);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE)
	{
		std::cout << "Frame buffer initialised successfully" << std::endl;
		frameBuffer->initialised = true;
	}
	else {
		std::cout << "Failed to initialise frame buffer" << std::endl;
		glDeleteFramebuffers(1, &frameBuffer->fbo);
		//glDeleteRenderbuffers(1, &frameBuffer->rbo);
		glDeleteTextures(1, &frameBuffer->texture.id);
		frameBuffer->initialised = false;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindTexture(GL_TEXTURE_2D, 0);
}


static void BindFrameBuffer(FrameBuffer* frameBuffer)
{
	glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer->fbo);
	glEnable(GL_DEPTH_TEST);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

static void ResolveFrameBuffer(FrameBuffer* input, FrameBuffer* output, float width, float height)
{
	glBindFramebuffer(GL_READ_FRAMEBUFFER, input->fbo);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, output->fbo);
	glBlitFramebuffer(0, 0, width, height, 0, 0, width, height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
}

static void UnbindFrameBuffer()
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

static void DrawFrameBuffer(ShaderProgram* shaderProgram, Mesh* mesh, FrameBuffer* frameBuffer, float left, float top, float width, float height)
{
	glDisable(GL_DEPTH_TEST);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	glUseProgram(shaderProgram->shaderProgram);
	SetUniform(shaderProgram, "u_left", left);
	SetUniform(shaderProgram, "u_top", top);
	SetUniform(shaderProgram, "u_width", width);
	SetUniform(shaderProgram, "u_height", height);
	BindTexture(&frameBuffer->texture, 0);
	DrawMesh(mesh);
	UnbindTexture();
}

static std::vector<char> GetDataFromFramBuffer(FrameBuffer* fb, GLsizei* stride)
{
	*stride = 4 * fb->width;
	GLsizei bufferSize = *stride * fb->width;
	std::vector<char> buffer(bufferSize);
	glPixelStorei(GL_PACK_ALIGNMENT, 4);
	glNamedFramebufferReadBuffer(fb->fbo, GL_FRONT);
	glReadPixels(0, 0, fb->width, fb->height, GL_RGBA, GL_UNSIGNED_BYTE, buffer.data());
	return buffer;
}

static void SaveImage(std::string path, char* data, int width, int height, int stride)
{
	//GLsizei stride;
	//std::vector<char> buffer = GetDataFromFramBuffer(fb, &stride);
	stbi_flip_vertically_on_write(true);
	stbi_write_png(path.c_str(), width, height, 4, data , stride);
}

static void InitSpriteRenderer(SpriteRenderer* spriteRenderer, int batchSize)
{
	spriteRenderer->maxVertices = batchSize * 4;
	spriteRenderer->maxIndices = batchSize * 6;
	spriteRenderer->maxTextureSlot = 32;

	// Gen buffer
	glGenBuffers(1, &spriteRenderer->vertexBuffer);
	glGenBuffers(1, &spriteRenderer->indexBuffer);
	glGenVertexArrays(1, &spriteRenderer->vao);

	spriteRenderer->vertices.resize(spriteRenderer->maxVertices);
	spriteRenderer->textureSlots.resize(spriteRenderer->maxTextureSlot);

	unsigned short* indices = new unsigned short[spriteRenderer->maxIndices];

	unsigned short offset = 0;
	for (int i = 0; i < spriteRenderer->maxIndices; i += 6)
	{
		indices[i + 0] = offset + 0;
		indices[i + 1] = offset + 1;
		indices[i + 2] = offset + 2;
		indices[i + 3] = offset + 1;
		indices[i + 4] = offset + 3;
		indices[i + 5] = offset + 2;
		offset += 4;
	}

	for (int i = 0; i < spriteRenderer->maxTextureSlot; i++)
	{
		std::stringstream tex;
		tex << "u_Textures[" << i << "]";
		SetUniform(spriteRenderer->program, tex.str(), i);
	}

	// white texture is the first texture
	// spriteRenderer->textureSlots[0] = textures[(int)TextureKey::white];
	spriteRenderer->textureSlotIndex = 0;

	// Bind buffers
	glBindBuffer(GL_ARRAY_BUFFER, spriteRenderer->vertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, spriteRenderer->maxVertices * sizeof(SpriteVertex), nullptr, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, spriteRenderer->indexBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, spriteRenderer->maxIndices * sizeof(unsigned short), indices, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	glBindVertexArray(spriteRenderer->vao);
	glBindBuffer(GL_ARRAY_BUFFER, spriteRenderer->vertexBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, spriteRenderer->indexBuffer);

	// Set vertex attributes 
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(SpriteVertex), (const void*)offsetof(SpriteVertex, SpriteVertex::position));

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(SpriteVertex), (const void*)offsetof(SpriteVertex, SpriteVertex::uv));

	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(SpriteVertex), (const void*)offsetof(SpriteVertex, SpriteVertex::color));

	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(SpriteVertex), (const void*)offsetof(SpriteVertex, SpriteVertex::textureIndex));

	glEnableVertexAttribArray(4);
	glVertexAttribPointer(4, 2, GL_FLOAT, GL_FALSE, sizeof(SpriteVertex), (const void*)offsetof(SpriteVertex, SpriteVertex::tiling));

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	delete[] indices;
}

static void ClearRenderer(SpriteRenderer* spriteRenderer)
{
	spriteRenderer->indexCount = 0;
	spriteRenderer->vertexCount = 0;
	spriteRenderer->textureSlotIndex = 0;
}

static void Render(SpriteRenderer* spriteRenderer)
{
	if (spriteRenderer->indexCount == 0) return;

	// bind the data
	glBindBuffer(GL_ARRAY_BUFFER, spriteRenderer->vertexBuffer);
	glBufferSubData(GL_ARRAY_BUFFER, 0, spriteRenderer->vertexCount * sizeof(SpriteVertex), spriteRenderer->vertices.data());
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	for (int i = 0; i < spriteRenderer->textureSlotIndex; i++)
	{
		glBindTextureUnit(i, spriteRenderer->textureSlots[i]->id);
	}

	glBindVertexArray(spriteRenderer->vao);
	glDrawElements(GL_TRIANGLES, spriteRenderer->indexCount, GL_UNSIGNED_SHORT, 0);

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}



static void AddSprite(SpriteRenderer* spriteRenderer, glm::mat4 transform, Texture* texture, glm::vec4 tintColor, glm::vec2 tiling, bool flipped)
{
	// if current batch reach max, render the current batch and clear it
	if (spriteRenderer->indexCount >= spriteRenderer->maxIndices)
	{
		Render(spriteRenderer);
		ClearRenderer(spriteRenderer);
	}

	int textureIndex = -1;
	for (int i = 0; i < spriteRenderer->textureSlotIndex; i++)
	{
		if (spriteRenderer->textureSlots[i]->id == texture->id)
		{
			textureIndex = i;
			break;
		}
	}

	if (textureIndex == -1)
	{
		// if there's a new texture and reach the max texture slot, render the current batch and clear it
		if (spriteRenderer->textureSlotIndex >= spriteRenderer->maxTextureSlot)
		{
			Render(spriteRenderer);
			ClearRenderer(spriteRenderer);
		}

		textureIndex = spriteRenderer->textureSlotIndex;
		spriteRenderer->textureSlots[spriteRenderer->textureSlotIndex] = texture;
		spriteRenderer->textureSlotIndex++;
	}

	for (int i = 0; i < 4; i++)
	{
		spriteRenderer->vertices[spriteRenderer->vertexCount].position = transform * quadPositions[i];
		spriteRenderer->vertices[spriteRenderer->vertexCount].uv = quadUvs[i];
		spriteRenderer->vertices[spriteRenderer->vertexCount].color = tintColor;
		spriteRenderer->vertices[spriteRenderer->vertexCount].textureIndex = textureIndex;
		spriteRenderer->vertices[spriteRenderer->vertexCount].tiling = tiling;

		if (flipped)
		{
			spriteRenderer->vertices[spriteRenderer->vertexCount].uv.x = 1 - spriteRenderer->vertices[spriteRenderer->vertexCount].uv.x;
		}
		spriteRenderer->vertexCount++;
	}
	spriteRenderer->indexCount += 6;
}

static void AddSprite(SpriteRenderer* spriteRenderer, Texture* texture, glm::mat4 transform, glm::vec4 color)
{
	 AddSprite(spriteRenderer, transform, texture, color, { 1, 1 }, false);
}

static glm::mat4 GetCameraProjection(Camera2D* camera)
{
	float heightInTiles = camera->windowSize.y / camera->zoom;
	float widthInTiles = camera->windowSize.x / camera->zoom;
	glm::vec2 posAfterZoom = camera->position / camera->zoom;
	return glm::ortho(
		-widthInTiles / 2.0f + posAfterZoom.x,
		widthInTiles / 2.0f + posAfterZoom.x,
		-heightInTiles / 2.0f + posAfterZoom.y,
		heightInTiles / 2.0f + posAfterZoom.y, -1.0f, 1.0f);

}
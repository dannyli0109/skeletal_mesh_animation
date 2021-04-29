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
};

struct Mesh
{
	GLuint vertexBuffer;
	GLuint indexBuffer;
	GLuint vao;
	unsigned int indexCount;
};

struct VertexData
{
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec3 vertTangent;
	glm::vec3 vertBitangent;
	glm::vec4 color;
	glm::vec2 uv;

	int boneIDs[MAX_BONE_INFLUENCE];
	float weights[MAX_BONE_INFLUENCE];
};

struct Bone
{
	int id;
	std::string name;
	glm::mat4 offset;
	std::vector<Bone> children;
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
static void LoadTexture(Texture* texture, std::string filename)
{
	int width, height, channels;
	unsigned char* data = stbi_load(filename.c_str(), &width, &height, &channels, 0);
	glGenTextures(1, &texture->id);
	glBindTexture(GL_TEXTURE_2D, texture->id);
	if (data)
	{
		if (channels == 3)
		{
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
			glTextureParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTextureParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glGenerateMipmap(GL_TEXTURE_2D);
		}
		else if (channels == 4)
		{
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
			glTextureParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTextureParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glGenerateMipmap(GL_TEXTURE_2D);
		}
		else {
			std::cout << "unknown texture type" << std::endl;
		}
	}
	else {
		std::cout << "Fail to load texture" << std::endl;
	}

	glBindTexture(GL_TEXTURE_2D, 0);
	stbi_image_free(data);
}
static void BindTexture(Texture* texture, int textureUnit)
{
	glActiveTexture(GL_TEXTURE0 + textureUnit);
	glBindTexture(GL_TEXTURE_2D, texture->id);
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

static MeshData LoadMeshData(const aiScene* scene)
{
	MeshData meshData = {};

	aiMesh* meshInfo = scene->mMeshes[0];
	meshData.vertices.reserve(meshInfo->mNumVertices);
	meshData.indices.reserve(meshInfo->mNumFaces);

	for (int i = 0; i < meshInfo->mNumVertices; i++)
	{
		VertexData newVertex = {};
		newVertex.position = { meshInfo->mVertices[i].x, meshInfo->mVertices[i].y, meshInfo->mVertices[i].z };

		if (meshInfo->HasVertexColors(0))
		{
			newVertex.color = { meshInfo->mColors[0][i].r, meshInfo->mColors[0][i].g, meshInfo->mColors[0][i].b, meshInfo->mColors[0][i].a };
		}
		else
		{
			newVertex.color = { 1, 1, 1, 1 };
		}

		if (meshInfo->HasTextureCoords(0))
		{
			newVertex.uv = { meshInfo->mTextureCoords[0][i].x, meshInfo->mTextureCoords[0][i].y };
		}
		else
		{
			newVertex.uv = { newVertex.position.x, newVertex.position.y };
		}

		if (meshInfo->HasNormals())
		{
			newVertex.normal = { meshInfo->mNormals[i].x, meshInfo->mNormals[i].y, meshInfo->mNormals[i].z };
		}
		else
		{
			newVertex.normal = { 0, 0, 0 };
		}

		if (meshInfo->HasTangentsAndBitangents())
		{
			newVertex.vertTangent = { meshInfo->mTangents[i].x, meshInfo->mTangents[i].y, meshInfo->mTangents[i].z };
			newVertex.vertBitangent = { meshInfo->mBitangents[i].x, meshInfo->mBitangents[i].y, meshInfo->mBitangents[i].z };
		}
		else
		{
			newVertex.vertTangent = { 0, 0, 0 };
			newVertex.vertBitangent = { 0, 0, 0 };
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

static MeshData LoadMeshData(const aiScene* scene, Bone& skeleton, int& boneCount)
{
	MeshData meshData = LoadMeshData(scene);
	aiMesh* meshInfo = scene->mMeshes[0];

	std::unordered_map<std::string, std::pair<int, glm::mat4>> boneInfo = {};
	std::vector<int> boneCounts;
	boneCounts.resize(meshData.vertices.size(), 0);
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
				meshData.vertices[id].boneIDs[0] = i;
				meshData.vertices[id].weights[0] = weight;
				break;
			case 1:
				meshData.vertices[id].boneIDs[1] = i;
				meshData.vertices[id].weights[1] = weight;
				break;
			case 2:
				meshData.vertices[id].boneIDs[2] = i;
				meshData.vertices[id].weights[2] = weight;
				break;
			case 3:
				meshData.vertices[id].boneIDs[3] = i;
				meshData.vertices[id].weights[3] = weight;
				break;
			default:
				break;
			}
			boneCounts[id]++;
		}
	}

	boneCount = meshInfo->mNumBones;
	ReadBone(skeleton, scene->mRootNode, boneInfo);
	return meshData;
}

static std::vector<Animation> LoadAnimations(const aiScene* scene, MeshData* meshData, Bone& skeleton, int& boneCount)
{
	std::vector<Animation> animations;
	aiAnimation** animationInfos = scene->mAnimations;
	aiNode* root = scene->mRootNode;

	for (int i = 0; i < scene->mNumAnimations; i++)
	{
		aiAnimation* anim = animationInfos[i];
		Animation animation = {};
		animation.duration = anim->mDuration;
		animation.ticksPersecond = anim->mTicksPerSecond;
		animation.globalInverseTransform = glm::inverse(ConvertAssimpToGLM(scene->mRootNode->mTransformation));
		animation.boneCount = boneCount;
		animation.skeleton = skeleton;

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
	int i = 0;
	return animations;
}

static std::pair<int, float> GetTimeFraction(std::vector<float>& times, float& dt) {
	int segment = 0;
	while (dt > times[segment])
		segment++;
	float start = times[segment - 1];
	float end = times[segment];
	float frac = (dt - start) / (end - start);
	return { segment, frac };
}


static void GetPose(Animation& animation, Bone& skeleton, float dt, glm::mat4& parentTransform)
{
	if (animation.boneTransforms.find(skeleton.name) == animation.boneTransforms.end()) return;
	BoneTransformTrack& btt = animation.boneTransforms[skeleton.name];
	dt = fmod(dt, animation.duration);
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
	animation.currentPose[skeleton.id] = animation.globalInverseTransform * globalTransform * skeleton.offset;

	for (Bone& child : skeleton.children)
	{
		GetPose(animation, child, dt, globalTransform);
	}
}

static void InitMesh(Mesh* mesh, MeshData* meshData)
{
	unsigned int indexCount = meshData->indices.size();
	unsigned int vertexCount = meshData->vertices.size();
	VertexData* vertices = meshData->vertices.data();
	unsigned short* indices = meshData->indices.data();

	*mesh = {};
	mesh->indexCount = indexCount;

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
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexData), (const void*)offsetof(VertexData, VertexData::position));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(VertexData), (const void*)offsetof(VertexData, VertexData::normal));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(VertexData), (const void*)offsetof(VertexData, VertexData::vertTangent));
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(VertexData), (const void*)offsetof(VertexData, VertexData::vertBitangent));
	glEnableVertexAttribArray(4);
	glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(VertexData), (const void*)offsetof(VertexData, VertexData::color));
	glEnableVertexAttribArray(5);
	glVertexAttribPointer(5, 2, GL_FLOAT, GL_FALSE, sizeof(VertexData), (const void*)offsetof(VertexData, VertexData::uv));
	
	glEnableVertexAttribArray(6);
	glVertexAttribPointer(6, 4, GL_FLOAT, GL_INT, sizeof(VertexData), (const void*)offsetof(VertexData, VertexData::boneIDs));
	glEnableVertexAttribArray(7);
	glVertexAttribPointer(7, 4, GL_FLOAT, GL_FALSE, sizeof(VertexData), (const void*)offsetof(VertexData, VertexData::weights));
	
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


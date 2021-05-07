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

struct VertexData
{
	union
	{
		struct {
			glm::vec3 position;
			glm::vec3 normal;
			glm::vec3 vertTangent;
			glm::vec3 vertBitangent;
			glm::vec4 color;
			glm::vec2 uv;
			int boneIDs[MAX_BONE_INFLUENCE];
			float weights[MAX_BONE_INFLUENCE];
		} mesh;

		struct
		{
			glm::vec3 position;
			glm::vec4 color;
		} line;
	};
};


struct Mesh
{
	GLuint vertexBuffer;
	GLuint indexBuffer;
	GLuint vao;
	std::vector<VertexData> vertices;
	unsigned int indexCount;
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

struct Material
{
	int type;
	union
	{
		struct {
			Texture diffuseTexture;
			Texture normalTexture;
			Texture specularTexture;
			Texture emissionTexture;
			glm::vec3 ka;
			glm::vec3 kd;
			glm::vec3 ks;
			glm::vec3 ke;
			float specularPower;
		} phong;

		struct {
			glm::vec3 color;
		} color;
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

static MeshData LoadMeshData(const aiScene* scene)
{
	MeshData meshData = {};

	aiMesh* meshInfo = scene->mMeshes[0];
	meshData.vertices.reserve(meshInfo->mNumVertices);
	meshData.indices.reserve(meshInfo->mNumFaces);

	for (int i = 0; i < meshInfo->mNumVertices; i++)
	{
		VertexData newVertex = {};
		newVertex.mesh.position = { meshInfo->mVertices[i].x, meshInfo->mVertices[i].y, meshInfo->mVertices[i].z };

		if (meshInfo->HasVertexColors(0))
		{
			newVertex.mesh.color = { meshInfo->mColors[0][i].r, meshInfo->mColors[0][i].g, meshInfo->mColors[0][i].b, meshInfo->mColors[0][i].a };
		}
		else
		{
			newVertex.mesh.color = { 1, 1, 1, 1 };
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
				meshData.vertices[id].mesh.boneIDs[0] = i;
				meshData.vertices[id].mesh.weights[0] = weight;
				break;
			case 1:
				meshData.vertices[id].mesh.boneIDs[1] = i;
				meshData.vertices[id].mesh.weights[1] = weight;
				break;
			case 2:
				meshData.vertices[id].mesh.boneIDs[2] = i;
				meshData.vertices[id].mesh.weights[2] = weight;
				break;
			case 3:
				meshData.vertices[id].mesh.boneIDs[3] = i;
				meshData.vertices[id].mesh.weights[3] = weight;
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

static std::vector<Animation> LoadAnimations(const aiScene* scene, Bone& skeleton, int& boneCount)
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
	mesh->vertices = meshData->vertices;

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
	glVertexAttribPointer(6, 4, GL_FLOAT, GL_INT, sizeof(VertexData), (const void*)offsetof(VertexData, VertexData::mesh.boneIDs));
	glEnableVertexAttribArray(7);
	glVertexAttribPointer(7, 4, GL_FLOAT, GL_FALSE, sizeof(VertexData), (const void*)offsetof(VertexData, VertexData::mesh.weights));
	
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


// Frame Buffer
static void InitFrameBuffer(FrameBuffer* frameBuffer, int width, int height)
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

	glBindTexture(GL_TEXTURE_2D, frameBuffer->texture.id);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer->fbo);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, frameBuffer->texture.id, 0);

	glBindRenderbuffer(GL_RENDERBUFFER, frameBuffer->rbo);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
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
}

static void BindFrameBuffer(FrameBuffer* frameBuffer)
{
	glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer->fbo);
	glEnable(GL_DEPTH_TEST);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
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

static void SaveImage(std::string path, GLFWwindow* window, FrameBuffer* fb)
{
	GLsizei nrChannels = 4;
	GLsizei stride = nrChannels * fb->width;
	GLsizei bufferSize = stride * fb->width;
	std::vector<char> buffer(bufferSize);
	glPixelStorei(GL_PACK_ALIGNMENT, 4);
	glNamedFramebufferReadBuffer(fb->fbo, GL_FRONT);

	glReadPixels(0, 0, fb->width, fb->height, GL_RGBA, GL_UNSIGNED_BYTE, buffer.data());
	stbi_flip_vertically_on_write(true);
	stbi_write_png(path.c_str(), fb->width, fb->height, nrChannels, buffer.data(), stride);
}
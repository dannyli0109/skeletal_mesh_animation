#include "ProgramManager.h"

int ProgramManager::Init()
{

    //Set resolution here, and give your window a different title.
    window = {};
    if (!InitWindow(&window, 1660, 760)) return -1;

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    resource = {};
    InitResources(&resource, &window);

    scene = {};
    InitScene(&scene, &resource, &window);

    input = {};
    lastInput = {};
    dt = 0;

    glfwSetWindowUserPointer(window.glfwWindow, &window);
    glfwSetWindowSizeCallback(window.glfwWindow, OnWindowResize);

    glfwSwapInterval(1);
    InitGUI(window.glfwWindow);
    return 1;
}

void ProgramManager::Update()
{
    while (!glfwWindowShouldClose(window.glfwWindow))
    {
        elapsedTime = (float)glfwGetTime();
        lastInput = input;
        input = {};
        HandleInput(window.glfwWindow, &input);

        if (window.width == 0 || window.height == 0) continue;

        HandleCameraController(&scene.camera, &input, &lastInput, window.glfwWindow, dt, 50000.0f, 2.0f);
        

        // draw frame buffer to screen
        
        //DrawFrameBuffer(
        //    &resource.shaders.output,
        //    &resource.meshes.quad,
        //    &resource.frameBuffers.output,
        //    0, 0,
        //    1.0f, 1.0f
        //);

        BeginRenderGUI();
        // begin imgui window
        RenderSceneWindow();
        RenderSpriteWindow();
        RenderSidePannel();
        RenderResourcePannel();
        EndRenderGUI();
        //Swapping the buffers – this means this frame is over.

        dt = (float)glfwGetTime() - elapsedTime;
        glfwSwapBuffers(window.glfwWindow);
    }
}

void ProgramManager::Destroy()
{
    DestroyResources(&resource, &window);
    glfwTerminate();
    DestroyGUI();
}

void ProgramManager::CaptureAnimationFrames()
{

    float timeIncrement = resource.animations.vampireAnimation.duration / (float)(frames);

    glViewport(0, 0, outputWidth, outputHeight);
    InitFrameBuffer(&resource.frameBuffers[MSAA_FRAMEBUFFER], outputWidth, outputHeight, msaa);
    InitFrameBuffer(&resource.frameBuffers[OUTPUT_FRAMEBUFFER], outputWidth, outputHeight);
    scene.camera.aspect = (float)outputWidth / (float)outputHeight;
    window.shouldUpdate = true;

    {
        glm::mat4 modelMatrix = GetModelMatrix(scene.models.positions[0], scene.models.rotations[0], scene.models.scales[0]);
        std::pair<glm::vec3, glm::vec3> volume = GetAnimationBoundingVolume(&resource.meshes[VAMPIRE_MESH], &resource.animations.vampireAnimation, modelMatrix, frames);
        SnapCameraToBoundingVolume(volume);
    }

    float frameTime = 0;
    std::vector<Texture> spriteTextures;
    std::string path = ".\\outputs";
    std::filesystem::create_directory(path);

    for (const auto& entry : std::filesystem::directory_iterator(path))
    {
        std::filesystem::remove_all(entry.path());
    }
    for (int i = 0; i < frames; i++)
    {
        BindFrameBuffer(&resource.frameBuffers[MSAA_FRAMEBUFFER]);
        for (int i = 0; i < resource.shaders.size(); i++)
        {
            UpdateScene(&resource.shaders[i], scene, frameTime);
        }
        UpdateModels(scene.models, frameTime);
        RenderModels(scene.models);
        //RenderLigths(&resource.shaders[COLOR_SHADER], resource, scene);

        ResolveFrameBuffer(&resource.frameBuffers[MSAA_FRAMEBUFFER], &resource.frameBuffers[OUTPUT_FRAMEBUFFER], outputWidth, outputHeight);
        UnbindFrameBuffer();
        // draw frame buffer to screen

        DrawFrameBuffer(
            &resource.shaders[OUTPUT_SHADER],
            &resource.meshes[QUAD_MESH],
            &resource.frameBuffers[OUTPUT_FRAMEBUFFER],
            0, 0,
            1.0f, 1.0f
        );

        std::string imagePath = path;
        imagePath += "\\frame";
        imagePath += std::to_string(i);
        imagePath += ".png";

        //ss << "outputs\\frame" << i << ".png";
        GLsizei stride;
        std::vector<char> buffer = GetDataFromFramBuffer(&resource.frameBuffers[OUTPUT_FRAMEBUFFER], &stride);
        SaveImage(imagePath.c_str(), buffer.data(), outputWidth, outputHeight, stride);
        {
            Texture texture;
            InitTexture(&texture, buffer.data(), outputWidth, outputHeight);
            spriteTextures.push_back(texture);
            //spriteTextures.push_back(texture);
        }
        frameTime += timeIncrement;
    }
    AddSpriteAnimation(&resource.spriteAnimations, spriteTextures, outputWidth, outputHeight, resource.animations.vampireAnimation.duration);
}

void ProgramManager::RenderBoundingVolume()
{
    ClearRenderer(&resource.lineRenderer);
    glUseProgram(resource.shaders[LINE_SHADER].shaderProgram);
    UpdateCamera(&resource.shaders[LINE_SHADER], scene.camera);

    AddLine(&resource.lineRenderer, { min.x, min.y, min.z }, { max.x, min.y, min.z }, { 1, 1, 0, 1 });
    AddLine(&resource.lineRenderer, { min.x, min.y, min.z }, { min.x, max.y, min.z }, { 1, 0, 1, 1 });
    AddLine(&resource.lineRenderer, { min.x, min.y, min.z }, { min.x, min.y, max.z }, { 0, 1, 1, 1 });

    AddLine(&resource.lineRenderer, { min.x, min.y, max.z }, { min.x, max.y, max.z }, { 1, 0, 0, 1 });

    AddLine(&resource.lineRenderer, { min.x, max.y, min.z }, { max.x, max.y, min.z }, { 1, 0, 0, 1 });
    AddLine(&resource.lineRenderer, { min.x, max.y, min.z }, { min.x, max.y, max.z }, { 1, 0, 0, 1 });

    AddLine(&resource.lineRenderer, { max.x, max.y, min.z }, { max.x, min.y, min.z }, { 1, 0, 0, 1 });

    AddLine(&resource.lineRenderer, { min.x, min.y, max.z }, { max.x, min.y, max.z }, { 1, 0, 0, 1 });
    AddLine(&resource.lineRenderer, { max.x, min.y, min.z }, { max.x, min.y, max.z }, { 1, 0, 0, 1 });

    AddLine(&resource.lineRenderer, { min.x, max.y, max.z }, { max.x, max.y, max.z }, { 1, 0, 0, 1 });
    AddLine(&resource.lineRenderer, { max.x, min.y, max.z }, { max.x, max.y, max.z }, { 1, 0, 0, 1 });
    AddLine(&resource.lineRenderer, { max.x, max.y, min.z }, { max.x, max.y, max.z }, { 1, 0, 0, 1 });
    Render(&resource.lineRenderer);
}

void ProgramManager::RenderSceneWindow()
{
    ImGui::Begin("Scene Window");
    ImVec2 size = ImGui::GetContentRegionAvail();
    if (size.x != window.width || size.y != window.height)
    {
        window.width = size.x;
        window.height = size.y;
        window.shouldUpdate = true;
    }

    if (window.shouldUpdate)
    {
        glViewport(0, 0, window.width, window.height);
        InitFrameBuffer(&resource.frameBuffers[MSAA_FRAMEBUFFER], window.width, window.height, msaa);
        InitFrameBuffer(&resource.frameBuffers[OUTPUT_FRAMEBUFFER], window.width, window.height);
        scene.camera.aspect = (float)window.width / (float)window.height;
        window.shouldUpdate = false;
    }

    else {
        glViewport(0, 0, window.width, window.height);
        glEnable(GL_DEPTH_TEST);
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        BindFrameBuffer(&resource.frameBuffers[MSAA_FRAMEBUFFER]);
        for (int i = 0; i < resource.shaders.size(); i++)
        {
            UpdateScene(&resource.shaders[i], scene, elapsedTime);
        }
        UpdateModels(scene.models, elapsedTime);
        RenderModels(scene.models);

        //RenderLigths(&resource.shaders[UNSHADED_SHADER], resource, scene);
        RenderBoundingVolume();

        ResolveFrameBuffer(&resource.frameBuffers[MSAA_FRAMEBUFFER], &resource.frameBuffers[OUTPUT_FRAMEBUFFER], window.width, window.height);
        UnbindFrameBuffer();
    }
    
    ImGui::Image((ImTextureID)resource.frameBuffers[OUTPUT_FRAMEBUFFER].texture.id, { size.x, size.y }, ImVec2(0, 1), ImVec2(1, 0));

    ImGui::End();
}

void ProgramManager::RenderSpriteWindow()
{
    ImGui::Begin("Sprite Window");
    ImVec2 size = ImGui::GetContentRegionAvail();
    if (size.x != scene.camera2D.windowSize.x || size.y != scene.camera2D.windowSize.y)
    {
        scene.camera2D.windowSize.x = size.x;
        scene.camera2D.windowSize.y = size.y;

        InitFrameBuffer(&resource.frameBuffers[SPRITE_FRAMEBUFFER], scene.camera2D.windowSize.x, scene.camera2D.windowSize.y);
    }

    if (resource.spriteAnimations.count > 0)
    {
        int index = resource.spriteAnimations.count - 1;
        int totalFrames = resource.spriteAnimations.textures[index].size();
        float deltaTime = fmod(elapsedTime, resource.spriteAnimations.durations[index]);
        int currentFrame = deltaTime / resource.spriteAnimations.durations[index] * totalFrames;
        float aspect = resource.spriteAnimations.widths[index] / resource.spriteAnimations.heights[index];
        resource.spriteAnimations.currentFrames[index] = currentFrame;

        BindFrameBuffer(&resource.frameBuffers[SPRITE_FRAMEBUFFER]);
        glViewport(0, 0, size.x, size.y);
        ClearRenderer(&resource.sprtieRenderer);
        glm::mat4 cameraProjection = GetCameraProjection(&scene.camera2D);
        SetUniform(&resource.shaders[SPRITE_SHADER], "u_ProjectionMatrix", cameraProjection);
        glm::mat4 spriteTransform = glm::mat4(1.0f);
        AddSprite(&resource.sprtieRenderer, spriteTransform, &resource.spriteAnimations.textures[index][currentFrame], { 1, 1, 1, 1 }, {1, 1}, false);

        Render(&resource.sprtieRenderer);
        UnbindFrameBuffer();

        ImGui::Image((ImTextureID)resource.frameBuffers[SPRITE_FRAMEBUFFER].texture.id, { size.x, size.y });
    }

    ImGui::End();
}

void ProgramManager::RenderSidePannel()
{
    ImGui::Begin("Imgui window");

    ImGui::DragFloat3("Camera Position", &scene.camera.position[0], 0.1f);
    float phi = glm::degrees(scene.camera.phi);
    ImGui::DragFloat("Camera Phi", &phi, 0.1f);
    scene.camera.phi = glm::radians(phi);

    float theta = glm::degrees(scene.camera.theta);
    ImGui::DragFloat("Camera Theta", &theta, 0.1f);
    scene.camera.theta = glm::radians(theta);

    ImGui::DragFloat("Camera Near Plane", &scene.camera.near, 0.1f);
    ImGui::DragFloat("Camera Far Plane", &scene.camera.far, 1.0f);

    ImGui::Spacing();
    glm::vec3 rot = glm::degrees(scene.models.rotations[0]);
    ImGui::DragFloat3("Rotation", &rot[0], 0.1f);
    scene.models.rotations[0] = glm::radians(rot);

   
    ImGui::InputFloat3("bounding min", &min.x);
    ImGui::InputFloat3("bounding max", &max.x);
    glm::vec3 diff = max - min;
    ImGui::InputFloat3("diff", &diff.x);
    //ImGui::DragFloat3()

    ImGui::InputInt("Width", &outputWidth);
    ImGui::InputInt("Height", &outputHeight);
    ImGui::InputInt("Frames", &frames);
    if (ImGui::Button("Generate bounding volume") && frames > 0)
    {
        glm::mat4 modelMatrix = GetModelMatrix(scene.models.positions[0], scene.models.rotations[0], scene.models.scales[0]);
        std::pair<glm::vec3, glm::vec3> volume = GetAnimationBoundingVolume(&resource.meshes[VAMPIRE_MESH], &resource.animations.vampireAnimation, modelMatrix, frames);
        SnapCameraToBoundingVolume(volume);
    }

    if (ImGui::Button("Capture") && frames > 0 && outputWidth > 0 && outputHeight > 0)
    {
        CaptureAnimationFrames();
    }

    ImGui::DragFloat("camera 2d zoom", &scene.camera2D.zoom);
    ImGui::End();
}

void ProgramManager::SnapCameraToBoundingVolume(std::pair<glm::vec3, glm::vec3> volume)
{
    min = volume.first;
    max = volume.second;
    scene.camera.position.x = (min.x + max.x) / 2.0f;
    scene.camera.position.y = (min.y + max.y) / 2.0f;

    float fovY = scene.camera.fovY;
    float fovX = 2 * atan(tanf(fovY * 0.5f) * scene.camera.aspect);
    float w = max.x - min.x;
    float h = max.y - min.y;
    float depth = max.z - min.z;
    float zX = (w / 2.0f) / tanf(fovX / 2.0f) + depth / 2.0f;
    float zY = (h / 2.0f) / tanf(fovY / 2.0f) + depth / 2.0f;

    float zCenter = (min.z + max.z) / 2.0f;
    scene.camera.position.z = fmaxf(zX, zY) + zCenter;

    scene.camera.near = scene.camera.position.z - zCenter - depth / 2.0f;
    scene.camera.far = scene.camera.position.z + depth / 2.0f;
}

void ProgramManager::RenderResourcePannel()
{
    ImGui::Begin("Resources");
    // load texture
    
    {
        // open Dialog Simple
        if (ImGui::Button("Load Texture"))
            ImGuiFileDialog::Instance()->OpenDialog("ChooseTextureDlgKey", "Choose File", ".png,.jpg", ".");

        // display
        if (ImGuiFileDialog::Instance()->Display("ChooseTextureDlgKey"))
        {
            // action if OK
            if (ImGuiFileDialog::Instance()->IsOk())
            {
                texturePathName += ImGuiFileDialog::Instance()->GetFilePathName();
            }

            // close
            ImGuiFileDialog::Instance()->Close();
        }

        ImGui::Text(texturePathName.c_str());
        ImGui::InputText("Texture Name", textureName, IM_ARRAYSIZE(textureName));
        if (ImGui::Button("Add"))
        {
            Texture texture = {};
            std::string texName(textureName);
            if (LoadTexture(&texture, texName, texturePathName))
            {
                resource.textures.push_back(texture);
            }
        }
    }

    // select model
    std::vector<std::string> modelNames = scene.models.names;
    if (scene.models.count > selectedModel)
    {
        ImGui::Combo("Models", &selectedModel, VectorOfStringGetter, static_cast<void*>(&modelNames), scene.models.count);
    }

    int selectedMesh = -1;
    for (int i = 0; i < resource.meshes.size(); i++)
    {
        if (&resource.meshes[i] == scene.models.meshes[selectedModel])
        {
            selectedMesh = i;
            break;
        }
    }

    std::vector<std::string> meshNames = MapArray<Mesh, std::string>(resource.meshes, [](Mesh mesh) {
        return mesh.name;
    });

    if (meshNames.size() > selectedMesh)
    {
        ImGui::Combo("Meshes", &selectedMesh, VectorOfStringGetter, static_cast<void*>(&meshNames), meshNames.size());
    }

    std::vector<std::string> materialNames = MapArray<Material, std::string>(resource.materials, [](Material material)
    {
            return material.name;
    });

    int selectedMaterial = -1;
    for (int i = 0; i < resource.materials.size(); i++)
    {
        if (&resource.materials[i] == scene.models.materials[selectedModel])
        {
            selectedMaterial = i;
            break;
        }
    }

    if (resource.materials.size() > selectedMaterial)
    {
        ImGui::Combo("Materials", &selectedMaterial, VectorOfStringGetter, static_cast<void*>(&materialNames), resource.materials.size());
    }


    //if (selectedMaterial == 1)
    //{
    //    int i = 0;
    //}

    /*
    	    Texture diffuseTexture;
			Texture normalTexture;
			Texture specularTexture;
			Texture emissionTexture;
			glm::vec3 ka;
			glm::vec3 kd;
			glm::vec3 ks;
			glm::vec3 ke;
			float specularPower;
    */
    ImGui::Spacing();
    switch (resource.materials[selectedMaterial].type)
    {
        case 0:
        {
            std::vector<std::string> textureNames = MapArray<Texture, std::string>(resource.textures, [](Texture texture) {
                return texture.name;
            });

            // diffuse
            {
                int selectedDiffusedTexture = -1;
                for (int i = 0; i < resource.textures.size(); i++)
                {
                    if (&resource.textures[i] == scene.models.materials[selectedModel]->phong.diffuseTexture)
                    {
                        selectedDiffusedTexture = i;
                        break;
                    }
                }
                if (resource.textures.size() > selectedDiffusedTexture && selectedDiffusedTexture != -1)
                {
                    ImGui::Combo("Diffuse Texture", &selectedDiffusedTexture, VectorOfStringGetter, static_cast<void*>(&textureNames), resource.textures.size());
                }

                scene.models.materials[selectedModel]->phong.diffuseTexture = &resource.textures[selectedDiffusedTexture];
            }

            // normal
            {
                int selectedNormalTexture = -1;
                for (int i = 0; i < resource.textures.size(); i++)
                {
                    if (&resource.textures[i] == scene.models.materials[selectedModel]->phong.normalTexture)
                    {
                        selectedNormalTexture = i;
                        break;
                    }
                }
                if (resource.textures.size() > selectedNormalTexture && selectedNormalTexture != -1)
                {
                    ImGui::Combo("Normal Texture", &selectedNormalTexture, VectorOfStringGetter, static_cast<void*>(&textureNames), resource.textures.size());
                }

                scene.models.materials[selectedModel]->phong.normalTexture = &resource.textures[selectedNormalTexture];
            }

            // specular
            {
                int selectedSpecularTexture = -1;
                for (int i = 0; i < resource.textures.size(); i++)
                {
                    if (&resource.textures[i] == scene.models.materials[selectedModel]->phong.specularTexture)
                    {
                        selectedSpecularTexture = i;
                        break;
                    }
                }
                if (resource.textures.size() > selectedSpecularTexture && selectedSpecularTexture != -1)
                {
                    ImGui::Combo("Specular Texture", &selectedSpecularTexture, VectorOfStringGetter, static_cast<void*>(&textureNames), resource.textures.size());
                }

                scene.models.materials[selectedModel]->phong.specularTexture = &resource.textures[selectedSpecularTexture];
            }

            // emission
            {
                int selectedEmissionTexture = -1;
                for (int i = 0; i < resource.textures.size(); i++)
                {
                    if (&resource.textures[i] == scene.models.materials[selectedModel]->phong.emissionTexture)
                    {
                        selectedEmissionTexture = i;
                        break;
                    }
                }
                if (resource.textures.size() > selectedEmissionTexture && selectedEmissionTexture != -1)
                {
                    ImGui::Combo("Emission Texture", &selectedEmissionTexture, VectorOfStringGetter, static_cast<void*>(&textureNames), resource.textures.size());
                }

                scene.models.materials[selectedModel]->phong.emissionTexture = &resource.textures[selectedEmissionTexture];
            }


            break;
        }
        case 1:
        {
            ImGui::ColorPicker3("Color", &resource.materials[selectedMaterial].color.color.x);
            break;
        }
        default:
            break;
    }


    scene.models.meshes[selectedModel] = &resource.meshes[selectedMesh];
    scene.models.materials[selectedModel] = &resource.materials[selectedMaterial];

    ImGui::End();
}

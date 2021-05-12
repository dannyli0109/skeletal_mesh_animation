#include "ProgramManager.h"

int ProgramManager::Init()
{

    //Set resolution here, and give your window a different title.
    window = {};
    if (!InitWindow(&window, 1280, 760)) return -1;

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

        HandleCameraController(&scene.camera, &input, &lastInput, window.glfwWindow, dt, 50.0f, 2.0f);

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
        EndRenderGUI();
        //Swapping the buffers – this means this frame is over.

        dt = (float)glfwGetTime() - elapsedTime;
        glfwSwapBuffers(window.glfwWindow);
    }
}

void ProgramManager::Destroy()
{
    glfwTerminate();
    DestroyGUI();
}

void ProgramManager::CaptureAnimationFrames()
{
    float timeIncrement = resource.animations.vampireAnimation.duration / (float)(frames);

    glViewport(0, 0, outputWidth, outputHeight);
    InitFrameBuffer(&resource.frameBuffers.msaa, outputWidth, outputHeight, msaa);
    InitFrameBuffer(&resource.frameBuffers.output, outputWidth, outputHeight);
    scene.camera.aspect = (float)outputWidth / (float)outputHeight;
    window.shouldUpdate = true;

    {
        glm::mat4 modelMatrix = GetModelMatrix(scene.models.positions[0], scene.models.rotations[0], scene.models.scales[0]);
        std::pair<glm::vec3, glm::vec3> volume = GetAnimationBoundingVolume(&resource.meshes.vampire, &resource.animations.vampireAnimation, modelMatrix, frames);
        SnapCameraToBoundingVolume(volume);
    }

    float frameTime = 0;
    std::vector<Texture> spriteTextures;
    for (int i = 0; i < frames; i++)
    {
        BindFrameBuffer(&resource.frameBuffers.msaa);
        UpdateScene(&resource.shaders.phong, scene, frameTime);
        RenderModels(&resource.shaders.phong, scene.models);
        RenderLigths(&resource.shaders.color, resource, scene);

        ResolveFrameBuffer(&resource.frameBuffers.msaa, &resource.frameBuffers.output, outputWidth, outputHeight);
        UnbindFrameBuffer();
        // draw frame buffer to screen

        DrawFrameBuffer(
            &resource.shaders.output,
            &resource.meshes.quad,
            &resource.frameBuffers.output,
            0, 0,
            1.0f, 1.0f
        );

        std::stringstream ss;
        ss << "outputs\\frame" << i << ".png";
        GLsizei stride;
        std::vector<char> buffer = GetDataFromFramBuffer(&resource.frameBuffers.output, &stride);
        SaveImage(ss.str(), buffer.data(), outputWidth, outputHeight, stride);
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
    glUseProgram(resource.shaders.line.shaderProgram);
    UpdateCamera(&resource.shaders.line, scene.camera);

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
        InitFrameBuffer(&resource.frameBuffers.msaa, window.width, window.height, msaa);
        InitFrameBuffer(&resource.frameBuffers.output, window.width, window.height);
        scene.camera.aspect = (float)window.width / (float)window.height;
        window.shouldUpdate = false;
    }

    else {
        glViewport(0, 0, window.width, window.height);
        glEnable(GL_DEPTH_TEST);
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        BindFrameBuffer(&resource.frameBuffers.msaa);
        UpdateScene(&resource.shaders.phong, scene, elapsedTime);
        RenderModels(&resource.shaders.phong, scene.models);
        RenderLigths(&resource.shaders.color, resource, scene);
        RenderBoundingVolume();

        ResolveFrameBuffer(&resource.frameBuffers.msaa, &resource.frameBuffers.output, window.width, window.height);
        UnbindFrameBuffer();
    }
    
    ImGui::Image((ImTextureID)resource.frameBuffers.output.texture.id, { size.x, size.y }, ImVec2(0, 1), ImVec2(1, 0));

    ImGui::End();
}

void ProgramManager::RenderSpriteWindow()
{
    ImGui::Begin("Sprite Window");
    if (resource.spriteAnimations.count > 0)
    {
        ImVec2 size = ImGui::GetContentRegionAvail();

        int index = resource.spriteAnimations.count - 1;
        int totalFrames = resource.spriteAnimations.textures[index].size();
        float dt = fmod(elapsedTime, resource.spriteAnimations.durations[index]);
        int currentFrame = dt / resource.spriteAnimations.durations[index] * totalFrames;
        float aspect = resource.spriteAnimations.widths[index] / resource.spriteAnimations.heights[index];

        ImGui::Image((ImTextureID)resource.spriteAnimations.textures[index][currentFrame].id, { size.x, size.x / aspect }, ImVec2(0, 1), ImVec2(1, 0));

        resource.spriteAnimations.currentFrames[index] = currentFrame;
    }
    ImGui::End();
}

void ProgramManager::RenderSidePannel()
{
    ImGui::Begin("Imgui window");

    {
        // open Dialog Simple
        if (ImGui::Button("Open File Dialog"))
            ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey", "Choose File", ".dae", ".");

        // display
        if (ImGuiFileDialog::Instance()->Display("ChooseFileDlgKey"))
        {
            // action if OK
            if (ImGuiFileDialog::Instance()->IsOk())
            {
                std::string filePathName = ImGuiFileDialog::Instance()->GetFilePathName();
                std::string filePath = ImGuiFileDialog::Instance()->GetCurrentPath();
                // action
                std::cout << filePath << std::endl;
                std::cout << filePathName << std::endl;
            }

            // close
            ImGuiFileDialog::Instance()->Close();
        }
    }

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
        std::pair<glm::vec3, glm::vec3> volume = GetAnimationBoundingVolume(&resource.meshes.vampire, &resource.animations.vampireAnimation, modelMatrix, frames);
        SnapCameraToBoundingVolume(volume);
    }

    if (ImGui::Button("Capture") && frames > 0 && outputWidth > 0 && outputHeight > 0)
    {
        CaptureAnimationFrames();
    }
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
    scene.camera.position.z = fmaxf(zX, zY);

    scene.camera.near = scene.camera.position.z - depth / 2.0f;
    scene.camera.far = scene.camera.position.z + depth / 2.0f;
}

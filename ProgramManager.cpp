#include "ProgramManager.h"

int ProgramManager::Init()
{

    //Set resolution here, and give your window a different title.
    window = {};
    if (!InitWindow(&window, 1280, 760)) return -1;

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    //glEnable(GL_MULTISAMPLE);

    //glEnable(GL_SCISSOR_TEST);

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
        float elapsedTime = (float)glfwGetTime();
        lastInput = input;
        input = {};
        HandleInput(window.glfwWindow, &input);

        if (window.width == 0 || window.height == 0) continue;
        if (window.shouldUpdate)
        {
            glViewport(0, 0, window.width, window.height);
            InitFrameBuffer(&resource.frameBuffers.output, window.width, window.height);
            scene.camera.aspect = (float)window.width / (float)window.height;
            window.shouldUpdate = false;
        }

        HandleCameraController(&scene.camera, &input, &lastInput, window.glfwWindow, dt, 50.0f, 2.0f);

        glViewport(0, 0, window.width, window.height);
        glEnable(GL_DEPTH_TEST);
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        BindFrameBuffer(&resource.frameBuffers.output);
        UpdateScene(&resource.shaders.phong, scene, elapsedTime);
        RenderModels(&resource.shaders.phong, scene.models);
        RenderLigths(&resource.shaders.color, resource, scene);

        RenderBoundingVolume();
        
        UnbindFrameBuffer();
        // draw frame buffer to screen
        
        DrawFrameBuffer(
            &resource.shaders.output,
            &resource.meshes.quad,
            &resource.frameBuffers.output,
            0, 0,
            1.0f, 1.0f
        );

        BeginRenderGUI();
        // begin imgui window
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

        ImGui::InputInt("Width", &outputWidth);
        ImGui::InputInt("Height", &outputHeight);
        ImGui::InputInt("Frames", &frames);
        if (ImGui::Button("Generate bounding volume") && frames > 0)
        {
            std::pair<glm::vec3, glm::vec3> volume = GetAnimationBoundingVolume(&resource.meshes.vampire, &resource.animations.vampireAnimation, frames);
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
            scene.camera.far = scene.camera.position.z + depth / 2.0f;
        }

        if (ImGui::Button("Capture") && frames > 0 && outputWidth > 0 && outputHeight > 0)
        {
            CaptureAnimationFrames();
        }
        ImGui::End();
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
    InitFrameBuffer(&resource.frameBuffers.output, outputWidth, outputHeight);
    scene.camera.aspect = (float)outputWidth / (float)outputHeight;
    window.shouldUpdate = true;

    {
        std::pair<glm::vec3, glm::vec3> volume = GetAnimationBoundingVolume(&resource.meshes.vampire, &resource.animations.vampireAnimation, frames);
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
        scene.camera.far = scene.camera.position.z + depth / 2.0f;
    }

    float frameTime = 0;
    for (int i = 0; i < frames; i++)
    {
        BindFrameBuffer(&resource.frameBuffers.output);
        UpdateScene(&resource.shaders.phong, scene, frameTime);
        RenderModels(&resource.shaders.phong, scene.models);
        RenderLigths(&resource.shaders.color, resource, scene);

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
        SaveImage(ss.str(), window.glfwWindow, &resource.frameBuffers.output);
        frameTime += timeIncrement;
    }

}

void ProgramManager::RenderBoundingVolume()
{
    ClearRenderer(&resource.lineRenderer);
    glUseProgram(resource.shaders.line.shaderProgram);
    UpdateCamera(&resource.shaders.line, scene.camera);

    AddLine(&resource.lineRenderer, { min.x, min.y, min.z }, { max.x, min.y, min.z }, { 1, 0, 0, 1 });
    AddLine(&resource.lineRenderer, { min.x, min.y, min.z }, { min.x, max.y, min.z }, { 1, 0, 0, 1 });
    AddLine(&resource.lineRenderer, { min.x, min.y, min.z }, { min.x, min.y, max.z }, { 1, 0, 0, 1 });

    AddLine(&resource.lineRenderer, { min.x, min.y, max.z }, { min.x, max.y, max.z }, { 1, 0, 0, 1 });
    AddLine(&resource.lineRenderer, { min.x, min.y, max.z }, { max.x, min.y, max.z }, { 1, 0, 0, 1 });

    AddLine(&resource.lineRenderer, { min.x, max.y, max.z }, { max.x, max.y, max.z }, { 1, 0, 0, 1 });

    AddLine(&resource.lineRenderer, { min.x, max.y, min.z }, { max.x, max.y, min.z }, { 1, 0, 0, 1 });
    AddLine(&resource.lineRenderer, { min.x, max.y, min.z }, { min.x, max.y, max.z }, { 1, 0, 0, 1 });


    AddLine(&resource.lineRenderer, { max.x, max.y, min.z }, { max.x, max.y, max.z }, { 1, 0, 0, 1 });
    AddLine(&resource.lineRenderer, { max.x, max.y, min.z }, { max.x, min.y, min.z }, { 1, 0, 0, 1 });

    AddLine(&resource.lineRenderer, { max.x, min.y, min.z }, { max.x, min.y, max.z }, { 1, 0, 0, 1 });
    AddLine(&resource.lineRenderer, { max.x, min.y, max.z }, { max.x, max.y, max.z }, { 1, 0, 0, 1 });
    Render(&resource.lineRenderer);
}

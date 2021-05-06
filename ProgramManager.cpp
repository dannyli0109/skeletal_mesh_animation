#include "ProgramManager.h"

int ProgramManager::Init()
{
    //Initialise GLFW, make sure it works. Put an error message here if you like.
    if (!glfwInit())
        return -1;

    //Set resolution here, and give your window a different title.
    window = {};
    window.glfwWindow = glfwCreateWindow(1280, 720, "Window", nullptr, nullptr);
    if (!window.glfwWindow)
    {
        glfwTerminate(); //Again, you can put a real error message here.
        return -1;
    }

    //This tells GLFW that the window we created is the one we should render to.
    glfwMakeContextCurrent(window.glfwWindow);

    //Tell GLAD to load all its OpenGL functions.
    if (!gladLoadGL())
        return -1;

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_SCISSOR_TEST);

    glfwGetWindowSize(window.glfwWindow, &window.width, &window.height);

    scene = {};
    {
        Camera camera = {};
        camera.position = { 0, 10000.0f, 30000.0f };
        camera.up = { 0, 1.0f, 0 };
        camera.theta = glm::radians(270.0f);
        camera.phi = glm::radians(0.0f);

        camera.fovY = glm::radians(45.0f);
        camera.aspect = (float)window.width / (float)window.height;
        camera.near = 1.0f;
        camera.far = 100000.0f;
        scene.camera = camera;
    }

    {
        ShaderProgram colorShader = {};
        InitShaderProgram(&colorShader, "Color.vert", "Color.frag");
        resourceManager.shaders.color = colorShader;
    }

    {
        ShaderProgram phongShader = {};
        InitShaderProgram(&phongShader, "Phong.vert", "Phong.frag");
        resourceManager.shaders.phong = phongShader;
        SetUniform(&resourceManager.shaders.phong, "u_diffuseTexture", 0);
        SetUniform(&resourceManager.shaders.phong, "u_normalTexture", 1);
        SetUniform(&resourceManager.shaders.phong, "u_specularTexture", 2);
        SetUniform(&resourceManager.shaders.phong, "u_emissionTexture", 3);
    }

    {
        ShaderProgram outputShader = {};
        InitShaderProgram(&outputShader, "Output.vert", "Output.frag");
        resourceManager.shaders.output = outputShader;
        SetUniform(&resourceManager.shaders.output, "u_colourTexture", 0);
    }

    {
        FrameBuffer fb = {};
        InitFrameBuffer(&fb, window.width, window.height);
        resourceManager.frameBuffers.output = fb;
    }

    {
        // init vampire
        Assimp::Importer importer;
        const aiScene* scene = importer.ReadFile("vampire/dancing_vampire.dae", aiProcess_Triangulate | aiProcess_JoinIdenticalVertices | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);
        
        Mesh vampireMesh = {};
        Bone vampireSkeleton = {};
        int boneCount = 0;
        MeshData vampireMeshData = LoadMeshData(scene, vampireSkeleton, boneCount);
        std::vector<Animation> animations = LoadAnimations(scene, &vampireMeshData, vampireSkeleton, boneCount);
        resourceManager.skeletons.vampireSkeleton = vampireSkeleton;

        resourceManager.animations.vampireAnimation = animations[0];
        InitMesh(&vampireMesh, &vampireMeshData);
        resourceManager.meshes.vampire = vampireMesh;
        Texture vampireDiffuseTexture = {};
        LoadTexture(&vampireDiffuseTexture, "vampire\\textures\\Vampire_diffuse.png");
        resourceManager.textures.vampireDiffuse = vampireDiffuseTexture;

        Texture vampireNormalTexture = {};
        LoadTexture(&vampireNormalTexture, "vampire\\textures\\Vampire_normal.png");
        resourceManager.textures.vampireNormal = vampireNormalTexture;

        Texture vampireSpecularTexture = {};
        LoadTexture(&vampireSpecularTexture, "vampire\\textures\\Vampire_specular.png");
        resourceManager.textures.vampireSpecular = vampireSpecularTexture;

        Texture vampireEmissionTexture = {};
        LoadTexture(&vampireEmissionTexture, "vampire\\textures\\Vampire_emission.png");
        resourceManager.textures.vampireEmission = vampireEmissionTexture;

        resourceManager.animations.vampireAnimation.currentPose.resize(boneCount, glm::mat4(1.0f));
    }

    {
        glm::mat4 modelMatrix = glm::scale(glm::mat4(1.0f), { 1.0f, 1.0f, 1.0f });
        Material material = {};
        material.phong.diffuseTexture = resourceManager.textures.vampireDiffuse;
        material.phong.normalTexture = resourceManager.textures.vampireNormal;
        material.phong.specularTexture = resourceManager.textures.vampireSpecular;
        material.phong.emissionTexture = resourceManager.textures.vampireEmission;
       
        material.phong.ka = { 1.0f, 1.0f, 1.0f };
        material.phong.kd = { 1.0f, 1.0f, 1.0f };
        material.phong.ks = { 1.0f, 1.0f, 1.0f };
        material.phong.ke = { 1.0f, 1.0f, 1.0f };
        material.phong.specularPower = 8.0f;

        AddModel(&scene, resourceManager.meshes.vampire, modelMatrix, 
            material,
            resourceManager.animations.vampireAnimation
        );
    }

    {
        Assimp::Importer importer;
        const aiScene* scene = importer.ReadFile("sphere.obj", aiProcess_Triangulate | aiProcess_JoinIdenticalVertices | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);
        Mesh sphereMesh = {};
        MeshData sphereMeshData = LoadMeshData(scene);
        InitMesh(&sphereMesh, &sphereMeshData);
        resourceManager.meshes.sphere = sphereMesh;
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
        resourceManager.meshes.quad = quadMesh;
    }

    {
        AmbientLight ambientLight = {};
        ambientLight.color = { 1.0f, 1.0f, 1.0f };
        ambientLight.intensity = 1.0f;
        scene.ambientLight = ambientLight;
    }

    {
        glm::vec3 lightPosition = { 0.0f, 100.0f, 100.0f };
        glm::vec3 lightColor = { 1.0f, 1.0f, 1.0f };
        float lightIntensity = 5000.0f;
        //AddPointLight(&scene, lightPosition, lightColor, lightIntensity);
    }

    input = {};
    lastInput = {};
    dt = 0;

    glfwSetWindowUserPointer(window.glfwWindow, &window);
    glfwSetWindowSizeCallback(window.glfwWindow, OnWindowResize);

    glfwSwapInterval(1);
    InitGUI(window.glfwWindow);
    return 0;
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
            InitFrameBuffer(&resourceManager.frameBuffers.output, window.width, window.height);
            scene.camera.aspect = (float)window.width / (float)window.height;
            window.shouldUpdate = false;
        }

        HandleCameraController(&scene.camera, &input, &lastInput, window.glfwWindow, dt, 50.0f, 2.0f);

        glViewport(0, 0, window.width, window.height);
        BindFrameBuffer(&resourceManager.frameBuffers.output);
        UpdateScene(&resourceManager.shaders.phong, scene, elapsedTime);
        RenderModels(&resourceManager.shaders.phong, scene.models);
        RenderLigths(&resourceManager.shaders.color, resourceManager, scene);
        UnbindFrameBuffer();
        // draw frame buffer to screen
        
        DrawFrameBuffer(
            &resourceManager.shaders.output,
            &resourceManager.meshes.quad,
            &resourceManager.frameBuffers.output,
            0, 0,
            1.0f, 1.0f
        );

        BeginRenderGUI();
        // begin imgui window
        ImGui::Begin("Imgui window");

        ImGui::DragFloat3("Camera Position", &scene.camera.position[0], 0.1f);

        ImGui::InputInt("Width", &outputWidth);
        ImGui::InputInt("Height", &outputHeight);
        ImGui::InputInt("Frames", &frames);

        if (ImGui::Button("capture") && frames > 0 && outputWidth > 0 && outputHeight > 0)
        {
            float timeIncrement = resourceManager.animations.vampireAnimation.duration / (float)(frames);

            glViewport(0, 0, outputWidth, outputHeight);
            InitFrameBuffer(&resourceManager.frameBuffers.output, outputWidth, outputHeight);
            scene.camera.aspect = (float)outputWidth / (float)outputHeight;
            window.shouldUpdate = true;

            for (int j = 0; j < 1; j++)
            {
                float frameTime = 0;
                for (int i = 0; i < frames; i++)
                {
                    BindFrameBuffer(&resourceManager.frameBuffers.output);
                    UpdateScene(&resourceManager.shaders.phong, scene, frameTime);
                    RenderModels(&resourceManager.shaders.phong, scene.models);
                    RenderLigths(&resourceManager.shaders.color, resourceManager, scene);
                    UnbindFrameBuffer();
                    // draw frame buffer to screen

                    DrawFrameBuffer(
                        &resourceManager.shaders.output,
                        &resourceManager.meshes.quad,
                        &resourceManager.frameBuffers.output,
                        0, 0,
                        1.0f, 1.0f
                    );

                    std::stringstream ss;
                    ss << "outputs\\frame" << i + j * frames << ".png";
                    SaveImage(ss.str(), window.glfwWindow, &resourceManager.frameBuffers.output);
                    frameTime += timeIncrement;
                }
            }            
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

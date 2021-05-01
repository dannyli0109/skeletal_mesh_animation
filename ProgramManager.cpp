#include "ProgramManager.h"

int ProgramManager::Init()
{
    //Initialise GLFW, make sure it works. Put an error message here if you like.
    if (!glfwInit())
        return -1;

    //Set resolution here, and give your window a different title.
    window = glfwCreateWindow(1280, 720, "Window", nullptr, nullptr);
    if (!window)
    {
        glfwTerminate(); //Again, you can put a real error message here.
        return -1;
    }

    //This tells GLFW that the window we created is the one we should render to.
    glfwMakeContextCurrent(window);

    //Tell GLAD to load all its OpenGL functions.
    if (!gladLoadGL())
        return -1;

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


    int width;
    int height;
    glfwGetWindowSize(window, &width, &height);

    {
        Camera camera = {};
        camera.position = { 0, 100.0f, 1000.0f };
        camera.up = { 0, 1.0f, 0 };
        camera.theta = glm::radians(270.0f);
        camera.phi = glm::radians(0.0f);

        camera.fovY = glm::radians(45.0f);
        camera.aspect = (float)width / (float)height;
        camera.near = 1.0f;
        camera.far = 2000.0f;
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
        InitFrameBuffer(&fb, width, height);
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
        glm::mat4 modelMatrix = glm::scale(glm::mat4(1.0f), { 0.01f, 0.01f, 0.01f });
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
        v0.position = { -0.5f, 0.5f, 0.5f };
        v0.uv = { 0, 0 };
        
        VertexData v1 = {};
        v1.position = { 0.5f, 0.5f, 0.5f };
        v1.uv = { 1.0f, 0 };

        VertexData v2 = {};
        v2.position = { -0.5f, -0.5f, 0.5f };
        v2.uv = { 0, 1.0f };

        VertexData v3 = {};
        v3.position = { 0.5f, -0.5f, 0.5f };
        v3.uv = { 1.0f, 1.0f };

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
    glfwSwapInterval(1);
    InitGUI(window);
    return 0;
}

void ProgramManager::Update()
{
    while (!glfwWindowShouldClose(window))
    {
        float elapsedTime = (float)glfwGetTime();
        lastInput = input;
        glfwPollEvents();
        double xPos, yPos;
        glfwGetCursorPos(window, &xPos, &yPos);
        input.mouseX = xPos;
        input.mouseY = yPos;
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) input.wKey = true;
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) input.aKey = true;
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) input.sKey = true;
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) input.dKey = true;

        HandleCameraController(&scene.camera, &input, &lastInput, window, dt, 1000.0f, 2.0f);

        BindFrameBuffer(&resourceManager.frameBuffers.output);
        UpdateScene(&resourceManager.shaders.phong, scene);
        RenderModels(&resourceManager.shaders.phong, scene.models);
        RenderLigths(&resourceManager.shaders.color, resourceManager, scene);
        UnbindFrameBuffer();

        // draw frame buffer to screen
        DrawFrameBuffer(
            &resourceManager.shaders.output, 
            &resourceManager.meshes.quad, 
            &resourceManager.frameBuffers.output
        );

        BeginRenderGUI();
        // begin imgui window
        ImGui::Begin("Imgui window");
        // draw ui element in between

        if (ImGui::Button("capture"))
        {
            SaveImage("outputs\\test.png", window, &resourceManager.frameBuffers.output);
        }
        ImGui::End();
        EndRenderGUI();
        //Swapping the buffers – this means this frame is over.

        dt = (float)glfwGetTime() - elapsedTime;
        glfwSwapBuffers(window);
    }
}

void ProgramManager::Destroy()
{
    glfwTerminate();
    DestroyGUI();
}

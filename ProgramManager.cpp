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

    glEnable(GL_DEPTH_TEST);


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

    ShaderProgram colorShader = {};
    InitShaderProgram(&colorShader, "Color.vert", "Color.frag");
    resourceManager.shaders.color = colorShader;

    ShaderProgram phongShader = {};
    InitShaderProgram(&phongShader, "Phong.vert", "Phong.frag");
    resourceManager.shaders.phong = phongShader;

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
        material.phong.specularPower = 256.0f;

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

    SetUniform(&resourceManager.shaders.phong, "u_diffuseTexture", 0);
    SetUniform(&resourceManager.shaders.phong, "u_normalTexture", 1);
    SetUniform(&resourceManager.shaders.phong, "u_specularTexture", 2);
    SetUniform(&resourceManager.shaders.phong, "u_emissionTexture", 3);

    InitGUI(window);
    return 0;
}

void ProgramManager::Update()
{
    while (!glfwWindowShouldClose(window))
    {
        float elapsedTime = (float)glfwGetTime();
        //Tell GLFW to check if anything is going on with input, etc.
        glfwPollEvents();
        //Clear the screen � eventually do rendering code here.
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        UpdateScene(&resourceManager.shaders.phong, scene);
        RenderModels(&resourceManager.shaders.phong, scene.models);

        RenderLigths(&resourceManager.shaders.color, resourceManager, scene);

        BeginRenderGUI();
        // begin imgui window
        ImGui::Begin("Imgui window");
        // draw ui element in between

        ImGui::End();
        EndRenderGUI();

        //Swapping the buffers � this means this frame is over.
        glfwSwapBuffers(window);
    }
}

void ProgramManager::Destroy()
{
    glfwTerminate();
    DestroyGUI();
}

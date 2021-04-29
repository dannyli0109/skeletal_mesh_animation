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

    camera = {};
    camera.position = { 0, 50.0f, 1000.0f };
    camera.up = { 0, 1.0f, 0 };
    camera.theta = glm::radians(270.0f);
    camera.phi = glm::radians(0.0f);

    camera.fovY = glm::radians(45.0f);
    camera.aspect = (float)width / (float)height;
    camera.near = 1.0f;
    camera.far = 2000.0f;

    resourceManager = {};
    
    ShaderProgram colorShader = {};
    InitShaderProgram(&colorShader, "Color.vert", "Color.frag");
    resourceManager.shaders.color = colorShader;

    ShaderProgram phongShader = {};
    InitShaderProgram(&phongShader, "Phong.vert", "Phong.frag");
    resourceManager.shaders.phong = phongShader;

    {
        Assimp::Importer importer;
        const aiScene* scene = importer.ReadFile("vampire/dancing_vampire.dae", aiProcess_Triangulate | aiProcess_JoinIdenticalVertices | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);
        
        Mesh vampireMesh = {};
        Bone vampireSkeleton = {};
        int boneCount = 0;
        MeshData vampireMeshData = LoadMeshData(scene, vampireSkeleton, boneCount);
        std::vector<Animation> animations = LoadAnimations(scene, &vampireMeshData);
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

        currentPose = {};
        currentPose.resize(boneCount, glm::mat4(1.0f));
        
    }

    {
        Assimp::Importer importer;
        const aiScene* scene = importer.ReadFile("sphere.obj", aiProcess_Triangulate | aiProcess_JoinIdenticalVertices | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);
        Mesh sphereMesh = {};
        MeshData sphereMeshData = LoadMeshData(scene);
        InitMesh(&sphereMesh, &sphereMeshData);
        resourceManager.meshes.sphere = sphereMesh;
    }



    PointLight pointLight = {};
    pointLight.position = { 0.0f, 100.0f, 100.0f };
    pointLight.color = { 1.0f, 1.0f, 1.0f };
    pointLight.intensity = 5000.0f;
     AddPointLight(&sceneManager, pointLight);

    SetUniform(&resourceManager.shaders.phong, "u_diffuseTexture", 0);
    SetUniform(&resourceManager.shaders.phong, "u_normalTexture", 1);
    SetUniform(&resourceManager.shaders.phong, "u_specularTexture", 2);

    // material
    SetUniform(&resourceManager.shaders.phong, "u_ka", {1, 1, 1});
    // SetUniform(&resourceManager.shaders.phong, "u_ambientLightIntensity", { 0.1f, 0.1f, 0.1f });
    SetUniform(&resourceManager.shaders.phong, "u_kd", {1.0f, 1.0f, 1.0f});
    SetUniform(&resourceManager.shaders.phong, "u_ks", {1.0f, 1.0f, 1.0f});
    SetUniform(&resourceManager.shaders.phong, "u_specularPower", 256.0f);

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
        //Clear the screen – eventually do rendering code here.
        glClearColor(0.5f, 0.5f, 0.5f, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        float time = glfwGetTime();
        
        glm::mat4 projectionMatrix = GetProjectionMatrix(&camera);
        glm::mat4 viewMatrix = GetViewMatrix(&camera);
        //glm::mat4 modelMatrix = glm::rotate(glm::mat4(1), time, { 0, 1, 0 });
        glm::mat4 modelMatrix = glm::scale(glm::mat4(1.0f), { 0.01f, 0.01f, 0.01f });
        
        SetUniform(&resourceManager.shaders.phong, "u_projectionMatrix", projectionMatrix);
        SetUniform(&resourceManager.shaders.phong, "u_viewMatrix", viewMatrix);
        SetUniform(&resourceManager.shaders.phong, "u_modelMatrix", modelMatrix);

        BindTexture(&resourceManager.textures.vampireDiffuse, 0);
        BindTexture(&resourceManager.textures.vampireNormal, 1);
        BindTexture(&resourceManager.textures.vampireSpecular, 2);

        for (int i = 0; i < sceneManager.pointlights.size(); i++)
        {
            std::stringstream lightPos;
            lightPos << "u_lightPositions[" << i << "]";
            std::stringstream lightIntensity;
            lightIntensity << "u_lightIntensities[" << i << "]";
            SetUniform(&resourceManager.shaders.phong, lightPos.str(), sceneManager.pointlights[i].position);
            SetUniform(&resourceManager.shaders.phong, lightIntensity.str(), sceneManager.pointlights[i].color * sceneManager.pointlights[i].intensity);
        }
        SetUniform(&resourceManager.shaders.phong, "u_lightCount", (int)sceneManager.pointlights.size());

        glm::mat4 parentTransform(1.0f);
        GetPose(resourceManager.animations.vampireAnimation, resourceManager.skeletons.vampireSkeleton, elapsedTime, currentPose, parentTransform);

        SetUniform(&resourceManager.shaders.phong, "u_boneTransforms", currentPose[0], currentPose.size());

      /*  for (int i = 0; i < resourceManager.animations.vampireAnimation.bones.size(); i++)
        {
            std::stringstream ss;
            ss << "u_finalBoneMatrices[" << i << "]";
            SetUniform(&resourceManager.shaders.phong, ss.str(), resourceManager.animations.vampireAnimation.bones[i].localTransform);
        }*/

        //DrawMesh(&resourceManager.meshes.soulSpear);
        DrawMesh(&resourceManager.meshes.vampire);


        glClear(GL_DEPTH_BUFFER_BIT);
        SetUniform(&resourceManager.shaders.color, "u_projectionMatrix", projectionMatrix);
        SetUniform(&resourceManager.shaders.color, "u_viewMatrix", viewMatrix);
        SetUniform(&resourceManager.shaders.color, "u_color", {0.0f, 1.0f, 0.0f});

 /*       for (int i = 0; i < resourceManager.animations.vampireAnimation.bones.size(); i++)
        {
            SetUniform(&resourceManager.shaders.color, "u_modelMatrix", resourceManager.animations.vampireAnimation.bones[i].localTransform);
            DrawMesh(&resourceManager.meshes.sphere);
        }*/

        BeginRenderGUI();
        RenderGUI();
        EndRenderGUI();

        //Swapping the buffers – this means this frame is over.
        glfwSwapBuffers(window);
        frame++;
    }
}

void ProgramManager::Destroy()
{
    glfwTerminate();
    DestroyGUI();
}

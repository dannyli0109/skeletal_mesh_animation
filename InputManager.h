#pragma once

struct Input
{
	double mouseX;
	double mouseY;
	short wKey;
	short aKey;
	short sKey;
	short dKey;
    float mouseScrollOffsetX;
    float mouseScrollOffsetY;
};

struct Window
{
	int width;
	int height;
	GLFWwindow* glfwWindow;
	bool shouldUpdate;
};

static int InitWindow(Window* window, int width, int height)
{
    //Initialise GLFW, make sure it works. Put an error message here if you like.
    if (!glfwInit())
        return -1;
    glfwWindowHint(GLFW_SAMPLES, 4);
    window->glfwWindow = glfwCreateWindow(width, height, "Window", nullptr, nullptr);

    if (!window->glfwWindow)
    {
        glfwTerminate(); //Again, you can put a real error message here.
        return -1;
    }

    //This tells GLFW that the window we created is the one we should render to.
    glfwMakeContextCurrent(window->glfwWindow);

    //Tell GLAD to load all its OpenGL functions.
    if (!gladLoadGL())
        return -1;

    glfwGetWindowSize(window->glfwWindow, &window->width, &window->height);
    return 1;
}

static void HandleInput(GLFWwindow* window, Input* input)
{
	glfwPollEvents();
    glfwGetCursorPos(window, &input->mouseX, &input->mouseY);
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) input->wKey = true;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) input->aKey = true;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) input->sKey = true;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) input->dKey = true;


}

#pragma once

struct Input
{
	double mouseX;
	double mouseY;
	short wKey;
	short aKey;
	short sKey;
	short dKey;
};

struct Window
{
	int width;
	int height;
	GLFWwindow* glfwWindow;
	bool shouldUpdate;
};

static void HandleInput(GLFWwindow* window, Input* input)
{
	glfwPollEvents();
    glfwGetCursorPos(window, &input->mouseX, &input->mouseY);
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) input->wKey = true;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) input->aKey = true;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) input->sKey = true;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) input->dKey = true;


}

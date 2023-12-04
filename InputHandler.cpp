#include "InputHandler.h"
#include <iostream>
#include <windows.h>
//#include <string.h>

void InputHandler::Init()
{
	position = glm::vec3(0.0f,4.0f,0.0f);
}

void InputHandler::ReactToEvents(GLFWwindow &window)
{

	float increaseValue = 0.01f;
	
	if (glfwGetKey(&window, GLFW_KEY_W) == GLFW_PRESS) 
	{
		position.y -= increaseValue;
	}
	if (glfwGetKey(&window, GLFW_KEY_S) == GLFW_PRESS)
	{
		position.y += increaseValue;
	}
	if (glfwGetKey(&window, GLFW_KEY_D) == GLFW_PRESS)
	{
		position.x -= increaseValue;
	}
	if (glfwGetKey(&window, GLFW_KEY_A) == GLFW_PRESS)
	{
		position.x += increaseValue;
	}
	if (glfwGetKey(&window, GLFW_KEY_UP) == GLFW_PRESS)
	{
		position.z += increaseValue;
	}
	if (glfwGetKey(&window, GLFW_KEY_DOWN) == GLFW_PRESS)
	{
		position.z -= increaseValue;
	}

	static bool debugInputPressed;
	if (glfwGetKey(&window, GLFW_KEY_GRAVE_ACCENT) == GLFW_PRESS)
	{
		debugInputPressed = true;
	}
	if (glfwGetKey(&window, GLFW_KEY_GRAVE_ACCENT) == GLFW_RELEASE)
	{
		if (debugInputPressed) 
		{
			isDebugEnabled = !isDebugEnabled;
			debugInputPressed = false;
		}
	}

}

void InputHandler::CompileShaders()
{
	system("GfxVulkanEngineCompileShaders.bat");
}

glm::vec3 InputHandler::GetPosition()
{
	return position;
}

bool InputHandler::IsDebugEnabled()
{
	return isDebugEnabled;
}

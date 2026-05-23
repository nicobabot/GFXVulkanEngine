#include "InputHandler.h"
#include <iostream>
#include <windows.h>
#include "utils.h"
//#include <string.h>

void InputHandler::Init()
{
	position = glm::vec3(0.0f, 2.0f, 5.0f);
	up = glm::vec3(0.0f, 1.0f, 0.0f);
	UpdateCameraVectors();
}

void InputHandler::UpdateCameraVectors()
{
	// Calculate the front vector from yaw and pitch
	glm::vec3 direction;
	direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	direction.y = sin(glm::radians(pitch));
	direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
	front = glm::normalize(direction);
	
	// Recalculate right and up vectors
	right = glm::normalize(glm::cross(front, glm::vec3(0.0f, 1.0f, 0.0f)));
	up = glm::normalize(glm::cross(right, front));
}

void InputHandler::ReactToEvents(GLFWwindow &window)
{
	// Handle WASD movement
	if (glfwGetKey(&window, GLFW_KEY_W) == GLFW_PRESS)
	{
		position += speed * front;
	}
	if (glfwGetKey(&window, GLFW_KEY_S) == GLFW_PRESS)
	{
		position -= speed * front;
	}
	if (glfwGetKey(&window, GLFW_KEY_A) == GLFW_PRESS)
	{
		position -= speed * right;
	}
	if (glfwGetKey(&window, GLFW_KEY_D) == GLFW_PRESS)
	{
		position += speed * right;
	}
	if (glfwGetKey(&window, GLFW_KEY_SPACE) == GLFW_PRESS)
	{
		position += speed * up;
	}
	if (glfwGetKey(&window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
	{
		position -= speed * up;
	}

	// Handle mouse input for camera look
	double mouseX, mouseY;
	glfwGetCursorPos(&window, &mouseX, &mouseY);
	
	bool isRightButtonPressed = glfwGetMouseButton(&window, GLFW_MOUSE_BUTTON_2);

	if(isRightButtonPressed)
	{
		if (firstMouse)
		{
			lastMouseX = mouseX;
			lastMouseY = mouseY;
			firstMouse = false;
		}
	
		double xOffset = mouseX - lastMouseX;
		double yOffset = lastMouseY - mouseY; // Reversed because y-coordinates go from bottom to top
	
		lastMouseX = mouseX;
		lastMouseY = mouseY;
	
		xOffset *= mouseSensitivity;
		yOffset *= mouseSensitivity;
	
		yaw += (float)xOffset;
		pitch += (float)yOffset;
	
		// Clamp pitch to prevent flipping
		if (pitch > 89.0f)
			pitch = 89.0f;
		if (pitch < -89.0f)
			pitch = -89.0f;

		UpdateCameraVectors();
	}
	else
	{
		lastMouseX = mouseX;
		lastMouseY = mouseY;
	}


	// Debug toggle
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

	if (glfwGetKey(&window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
	{
		wantToExit = true;
	}
}

void InputHandler::CompileShaders()
{
	std::string file = "GfxVulkanEngineCompileShaders_DXC.bat";
	if (FileExists(file))
	{
		system("GfxVulkanEngineCompileShaders_DXC.bat");
	}
}

glm::vec3 InputHandler::GetPosition()
{
	return position;
}

glm::vec3 InputHandler::GetFront()
{
	return front;
}

bool InputHandler::IsDebugEnabled()
{
	return isDebugEnabled;
}

bool InputHandler::WantToExit()
{
	return wantToExit;
}

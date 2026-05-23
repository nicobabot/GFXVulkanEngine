#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
//#include "gfxMaths.h"

class InputHandler
{
	public:
	void Init();
	void ReactToEvents(GLFWwindow& window);
	void CompileShaders();
	glm::vec3 GetPosition();
	glm::vec3 GetFront();
	bool IsDebugEnabled();
	bool WantToExit();

	private:
	glm::vec3 position;
	glm::vec3 front;
	glm::vec3 right;
	glm::vec3 up;
	
	float yaw = -90.0f;   // Start looking forward
	float pitch = 0.0f;
	float speed = 0.05f;
	float mouseSensitivity = 0.1f;
	
	double lastMouseX = 400.0;
	double lastMouseY = 300.0;
	bool firstMouse = true;
	
	bool isDebugEnabled = false;
	bool wantToExit = false;
	
	void UpdateCameraVectors();
};


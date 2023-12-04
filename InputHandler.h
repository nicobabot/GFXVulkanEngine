#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
//#include "gfxMaths.h"

class InputHandler
{
	public:
	void Init();
	void ReactToEvents(GLFWwindow& window);
	void CompileShaders();
	glm::vec3 GetPosition();
	bool IsDebugEnabled();

	private:
	glm::vec3 position;
	bool isDebugEnabled = false;
};


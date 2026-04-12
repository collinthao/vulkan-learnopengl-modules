#include "./windowContext.h"
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <memory>

class GLFWWindowContext : public IWindowContext
{
	public:
	GLFWWindowContext();
	bool framebufferResized = false;
	uint32_t WIDTH = 800;
	uint32_t HEIGHT = 600;
	GLFWwindow* window;

	
	void initWindow();
	void static framebufferResizeCallback(GLFWwindow* window, int width, int height);
	void static mouse_callback(GLFWwindow * window, double xpos, double ypos);
};

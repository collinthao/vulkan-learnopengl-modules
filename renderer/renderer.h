#ifndef RENDERER_H
#define RENDERER_H
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

class IRenderer
{
	public:
	virtual ~IRenderer(){};
	virtual void init(GLFWwindow* window) = 0;
	virtual void drawFrame() = 0;
};

#endif

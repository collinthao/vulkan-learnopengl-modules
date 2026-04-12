#ifndef ENGINE_H
#define ENGINE_H
#include "../renderer/vulkanRenderer.h"
#include "../windowContext/glfwWindowContext.h"
#include <memory>

class Engine
{
	public:
	Engine();
	std::unique_ptr<IRenderer> renderer = std::make_unique<VulkanRenderer>();
	std::unique_ptr<IWindowContext> windowContext = std::make_unique<GLFWWindowContext>();
	
	void init();
};

#endif

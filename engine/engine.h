#ifndef ENGINE_H
#define ENGINE_H
#include "../renderer/vulkanRenderer.h"
#include "../windowContext/glfwWindowContext.h"
#include <memory>

class Engine
{
	public:
	Engine();
	std::unique_ptr<IWindowContext> windowContext = std::make_unique<GLFWWindowContext>();
	std::unique_ptr<IRenderer> renderer = std::make_unique<VulkanRenderer>(static_cast<GLFWwindow*>(windowContext->getWindow()));
	
	void start();
};

#endif

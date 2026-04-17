#ifndef VULKANRENDERER_H
#define VULKANRENDERER_H
#include "./renderer.h"
#include "./VulkanApp.h"

class VulkanRenderer : public IRenderer
{


	private:
	VulkanApp vkApp = VulkanApp();
	void drawFrame(GLFWwindow * window);
	
	public:
	VulkanRenderer();
	VulkanRenderer(GLFWwindow * window);
	void render(GLFWwindow * window);
	void processInput(GLFWwindow * window);
	void cleanup(GLFWwindow * window);
};

#endif

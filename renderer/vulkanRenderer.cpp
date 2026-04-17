#include "./vulkanRenderer.h"

VulkanRenderer::VulkanRenderer(GLFWwindow * window)
{
	vkApp.init(window);
};

void VulkanRenderer::render(GLFWwindow * window)
{
	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();
		processInput(window);
		drawFrame(window);
	}

	cleanup(window);

	vkApp.deviceWaitIdle();
}

void VulkanRenderer::drawFrame(GLFWwindow * window)
{
	vkApp.drawFrame(window);
}


void VulkanRenderer::processInput(GLFWwindow * window)
{
	vkApp.processInput(window);
}

void VulkanRenderer::cleanup(GLFWwindow * window)
{
	vkApp.cleanup(window);
}

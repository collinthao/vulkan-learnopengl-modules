#include "./glfwWindowContext.h"
#include <iostream>

GLFWWindowContext::GLFWWindowContext(){};

void GLFWWindowContext::initWindow()
{
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

	window = glfwCreateWindow(800, 600, "Vulkan", nullptr, nullptr);
	glfwSetWindowUserPointer(window, this);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
};

void GLFWWindowContext::mouse_callback(GLFWwindow * window, double xpos, double ypos)
{
	std::cout << xpos << '\n';
}

void GLFWWindowContext::framebufferResizeCallback(GLFWwindow* window, int width, int height)
{
	auto windowContext = reinterpret_cast<GLFWWindowContext*>(glfwGetWindowUserPointer(window));
	windowContext->framebufferResized = true;
}


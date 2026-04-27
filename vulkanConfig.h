#ifndef VULKANCONFIG_H
#define VULKANCONFIG_H
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace VulkanConfig
{
	extern VkExtent2D swapChainExtent;
	extern const int MAX_FRAMES_IN_FLIGHT;
};

#endif

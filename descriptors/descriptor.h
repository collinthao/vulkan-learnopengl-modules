#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

class Descriptor
{
	public:
	Descriptor(VkDescriptorSetLayout layout);
	VkDescriptorSetLayout layout;
	VkDescriptorPool 	pool;
};

#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

class Descriptor
{
	public:
	Descriptor();
	Descriptor(VkDescriptorSetLayout layout, VkDescriptorPool pool);
	VkDescriptorSetLayout layout;
	VkDescriptorPool 	pool;
};

#pragma once
#include <vector>
#include "descriptor.h"
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace Descriptors
{
	class Builder
	{
		public:
		virtual ~Builder(){};
		virtual Descriptor build(VkDevice& device) = 0;
		virtual Builder& setBindings(std::vector<VkDescriptorSetLayoutBinding> bindings) = 0;
		virtual Builder& setPool() = 0;
	};
};

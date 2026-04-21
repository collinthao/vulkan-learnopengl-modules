#pragma once
#include <array>
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
		virtual Builder& setBindings(std::array<VkDescriptorSetLayoutBinding, 2> bindings) = 0;
		virtual Builder& setPool() = 0;
	};
};

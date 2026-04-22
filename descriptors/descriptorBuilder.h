#pragma once
#include "./builder.h"

class DescriptorBuilder : protected Descriptors::Builder
{
	public:
	std::vector<VkDescriptorSetLayoutBinding> bindings;

	Builder& setBindings(std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings)
	{
		bindings = setLayoutBindings;		
		return *this;
	};

	Builder& setPool()
	{
		return *this;
	}

	Descriptor build(VkDevice& device)
	{
		VkDescriptorSetLayout descriptorSetLayout;

		for (size_t i = 0; i < bindings.size(); i++)
		{
			bindings[i].binding = i;
		}

		VkDescriptorSetLayoutCreateInfo layoutInfo{};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
		layoutInfo.pBindings = bindings.data();

		if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create descriptor set layout!");
		}

		return Descriptor(descriptorSetLayout);
	}
};

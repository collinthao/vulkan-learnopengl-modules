#pragma once
#include <string>
#include <array>
#include <vector>
#include "./pipeline.h"
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include "../descriptors/descriptorBuilder.h"


class Builder
{
	protected:
	virtual VkPipelineLayout createPipelineLayout(VkDevice& device, VkPipelineLayout pipelineLayout) = 0;
	virtual VkShaderModule createShaderModule(const std::vector<char>& code, VkDevice& device) = 0;

	public:
	virtual ~Builder(){};

	virtual Builder& setShaderPaths(std::string vertPath, std::string fragPath) = 0;
	virtual Builder& setBindingDescription(VkVertexInputBindingDescription description) = 0;
	virtual Builder& setAttributeDescriptions(std::array<VkVertexInputAttributeDescription, 4> descriptions) = 0;
	virtual Builder& setTopology(VkPrimitiveTopology topology) = 0;
	virtual Builder& setMSAASamples(VkSampleCountFlagBits samples) = 0;
	virtual Builder& setDescriptorSetLayout(VkDescriptorSetLayout layouts) = 0;
	virtual Builder& setDescriptor(std::vector<VkDescriptorSetLayoutBinding> bindings, std::vector<VkDescriptorType> types, VkDevice& device) = 0;
	virtual Builder& setStencilTest(VkBool32 stencilTest) = 0;
	virtual Builder& setStencilState(VkStencilOp fail, VkStencilOp pass, VkStencilOp depthFail, VkCompareOp compare) = 0;
	virtual Builder& setStencilWriteMask(uint32_t mask) = 0;
	virtual Builder& setDepthTest(VkBool32 depthTest) = 0;
	virtual Builder& setDepthWrite(VkBool32 depthWrite) = 0;
	virtual Builder& setDepthCompareOp(VkCompareOp compareOp) = 0;
	virtual Builder& setRenderPass(VkRenderPass pass) = 0;
	virtual Builder& setCullMode(VkCullModeFlags mode) = 0;
	virtual Builder& setCullFace(VkFrontFace face) = 0;
	virtual Pipeline build(VkDevice& device) = 0;
};

#ifndef BUILDER_H 
#define BUILDER_H
#include <string>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>


class Builder
{
	public:
	virtual ~Builder(){};

	virtual void setShaderStages(std::string vertPath, std::string fragPath) = 0;

	virtual void setBindingDescription(VkVertexInputBindingDescription& bindingDescription) = 0;
	virtual void setAttributeDescriptions(VkVertexInputAttributeDescription& attributeDescriptions) = 0;
	virtual void setTopology(VkPrimitiveTopology topology) = 0;
	virtual void setMSAASamples(VkSampleCountFlagBits msaaSamples);
	virtual void setDescriptorSetLayout(VkDescriptorSetLayout* setLayouts) = 0;
	virtual void setStencilTest(VkBool32 stencilTestEnable) = 0;
	virtual void setStencilState(VkStencilOp failOp, VkStencilOp passOp, VkStencilOp depthFailOp, VkCompareOp compareOp) = 0;
	virtual void setDepthTest(VkBool32 depthTestEnable) = 0;
	virtual void setDepthWrite(VkBool32 depthWriteEnable) = 0;
};

#endif

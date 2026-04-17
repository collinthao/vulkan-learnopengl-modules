#ifndef PIPELINE_H
#define PIPELINE_H
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

class Pipeline
{
	private:
	VkPipelineShaderStageCreateInfo shaderStages[2];
	VkVertexInputAttributeDescription attributeDescriptions;
	VkPrimitiveTopology topology;
	VkSampleCountFlagBits msaaSamples;
	VkDescriptorSetLayout* setLayouts;
	VkBool32 stencilTestEnable;
	VkBool32 depthTestEnable;
	VkBool32 depthWriteEnable;
	VkStencilOp failOp; 
	VkStencilOp passOp; 
	VkStencilOp depthFailOp; 
	VkCompareOp compareOp;

	public:
	Pipeline();
	VkPipelineLayout pipelineLayout;
	VkPipeline pipeline;

	void createPipelineLayout(VkDevice& device); 
	void createPipeline(VkDevice& device); 
};

#endif

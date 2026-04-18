#ifndef PIPELINE_H
#define PIPELINE_H
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <array>

class Pipeline
{
	public:
	Pipeline();
	Pipeline(VkDevice& device, VkPipeline pipeline, VkPipelineLayout pipelineLayout);
	VkDevice device;
	VkPipelineLayout pipelineLayout;
	VkPipeline pipeline;

	VkPipelineLayout& getLayout();
	void bind(VkCommandBuffer cmd);
};

#endif

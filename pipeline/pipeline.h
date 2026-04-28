#ifndef PIPELINE_H
#define PIPELINE_H
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <array>
#include "../descriptors/descriptor.h"

class Pipeline
{
	public:
	Pipeline();
	Pipeline(VkDevice& device, VkPipeline pipeline, VkPipelineLayout pipelineLayout, Descriptor descriptor);
	VkDevice device;
	VkPipelineLayout pipelineLayout;
	VkPipeline pipeline;
	Descriptor descriptor;

	VkPipelineLayout& getLayout();
	void bind(VkCommandBuffer cmd);
};

#endif

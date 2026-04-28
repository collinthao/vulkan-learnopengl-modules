#include "pipeline.h"

Pipeline::Pipeline(){};

Pipeline::Pipeline(VkDevice& device, VkPipeline pipeline, VkPipelineLayout pipelineLayout, Descriptor descriptor)
: device(device), pipeline(pipeline), pipelineLayout(pipelineLayout), descriptor(descriptor)
{};

VkPipelineLayout& Pipeline::getLayout()
{
	return pipelineLayout;
}

void Pipeline::bind(VkCommandBuffer cmd)
{
	vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);	
}

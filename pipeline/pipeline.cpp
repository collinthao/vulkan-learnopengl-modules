#include "pipeline.h"

Pipeline::Pipeline(){};

Pipeline::Pipeline(VkDevice& device, VkPipeline pipeline, VkPipelineLayout pipelineLayout)
: device(device), pipeline(pipeline), pipelineLayout(pipelineLayout)
{};

VkPipelineLayout& Pipeline::getLayout()
{
	return pipelineLayout;
}

void Pipeline::bind(VkCommandBuffer cmd)
{
	vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);	
}

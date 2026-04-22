#ifndef PIPELINEBUILDER_H
#define PIPELINEBUILDER_H
#include "./builder.h"
#include <stdexcept>
#include "../vulkanConfig.h"
#include "../fileContext.h"

class PipelineBuilder : private Builder
{
	
	private: 
	Descriptor descriptor;
	std::string shaderPaths[2];
	VkVertexInputBindingDescription bindingDescription;
	std::array<VkVertexInputAttributeDescription, 4> attributeDescriptions;
	VkPrimitiveTopology primitiveTopology;
	VkSampleCountFlagBits msaaSamples;
	VkDescriptorSetLayout setLayouts;
	VkBool32 stencilTestEnable;
	VkBool32 depthTestEnable;
	VkBool32 depthWriteEnable;
	VkStencilOp failOp; 
	VkStencilOp passOp; 
	uint32_t stencilWriteMask; 
	VkStencilOp depthFailOp; 
	VkCompareOp compareOp;
	VkRenderPass renderPass;
	VkFrontFace cullFace;
	VkCullModeFlags cullMode;
	VkCompareOp depthCompareOp;

	VkShaderModule createShaderModule(const std::vector<char>& code, VkDevice& device)
	{
		VkShaderModuleCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.codeSize = code.size();
		createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());
		
		VkShaderModule shaderModule;
		if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create shader module");
		}

		return shaderModule;
	}

	VkPipelineLayout createPipelineLayout(VkDevice& device, VkPipelineLayout pipelineLayout)
	{
		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 1;
		pipelineLayoutInfo.pSetLayouts = &setLayouts;
		pipelineLayoutInfo.pushConstantRangeCount = 0;
		pipelineLayoutInfo.pPushConstantRanges = nullptr;

		std::cout << "create pipeline layout...\n";
		if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create pipeline layout!");
		}
		return pipelineLayout;
	}

	public:
	PipelineBuilder(){};

	PipelineBuilder& setDescriptor(std::vector<VkDescriptorSetLayoutBinding> bindings)
	{
		return *this;
	}

	PipelineBuilder& setDepthCompareOp(VkCompareOp compareOp)
	{
		depthCompareOp = compareOp;
		return *this;
	};

	PipelineBuilder& setCullFace(VkFrontFace face)
	{
		cullFace = face;
		return *this;
	}


	PipelineBuilder& setCullMode(VkCullModeFlags mode)
	{
		cullMode = mode;
		return *this;
	}

	PipelineBuilder& setShaderPaths(std::string vertPath, std::string fragPath)
	{
		shaderPaths[0] = vertPath;
		shaderPaths[1] = fragPath;
		return *this;
	};

	PipelineBuilder& setBindingDescription(VkVertexInputBindingDescription description)
	{
		bindingDescription = description;
		return *this;
	};
	PipelineBuilder& setAttributeDescriptions(std::array<VkVertexInputAttributeDescription, 4> descriptions)
	{
		attributeDescriptions = descriptions;
		return *this;
	};
	PipelineBuilder& setTopology(VkPrimitiveTopology topology)
	{
		primitiveTopology = topology;
		return *this;
	};
	PipelineBuilder& setMSAASamples(VkSampleCountFlagBits samples)
	{
		msaaSamples = samples;
		return *this;
	};
	PipelineBuilder& setDescriptorSetLayout(VkDescriptorSetLayout layouts)
	{
		setLayouts = layouts;
		return *this;
	};
	PipelineBuilder& setStencilTest(VkBool32 stencilTest)
	{
		stencilTestEnable = stencilTest;
		return *this;
	};

	PipelineBuilder& setStencilState(VkStencilOp fail, VkStencilOp pass, VkStencilOp depthFail, VkCompareOp compare)
	{
		failOp = fail;
		passOp = pass;
		depthFailOp = depthFail;
		compareOp = compare;
		return *this;
	};

	PipelineBuilder& setStencilWriteMask(uint32_t mask)
	{
		stencilWriteMask = mask;
		return *this;
	};

	PipelineBuilder& setDepthTest(VkBool32 depthTest)
	{
		depthTestEnable = depthTest;
		return *this;
	};
	PipelineBuilder& setDepthWrite(VkBool32 depthWrite)
	{
		depthWriteEnable = depthWrite;
		return *this;
	};
	PipelineBuilder& setRenderPass(VkRenderPass pass)
	{
		renderPass = pass;
		return *this;
	};

	Pipeline build(VkDevice& device)
	{	
		VkPipeline pipeline;
		VkPipelineLayout pipelineLayout{}; 
		pipelineLayout = createPipelineLayout(device, pipelineLayout);

		auto vertShaderCode = FileContext::readFile(shaderPaths[0]);
		auto fragShaderCode = FileContext::readFile(shaderPaths[1]);

		VkShaderModule vertShaderModule = createShaderModule(vertShaderCode, device);
		VkShaderModule fragShaderModule = createShaderModule(fragShaderCode, device);

		VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
		vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
		vertShaderStageInfo.module = vertShaderModule;
		vertShaderStageInfo.pName = "main";
		
		VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
		fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		fragShaderStageInfo.module = fragShaderModule;
		fragShaderStageInfo.pName = "main";

		VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

		auto bindingDescription = Vertex::getBindingDesciption();
		auto attributeDescriptions = Vertex::getAttributeDescriptions();

		VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
		vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputInfo.vertexBindingDescriptionCount = 1;
		vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
		vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
		vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

		VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
		inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssembly.topology = primitiveTopology;
		inputAssembly.primitiveRestartEnable = VK_FALSE;

		VkViewport viewport{};
		viewport.x = 0.f;
		viewport.y = 0.f;
		viewport.width = (float)VulkanConfig::swapChainExtent.width;
		viewport.height = (float)VulkanConfig::swapChainExtent.height;
		viewport.minDepth = 0.f;
		viewport.maxDepth = 1.f;

		VkPipelineViewportStateCreateInfo viewportState{};
		viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportState.viewportCount = 1;
		viewportState.scissorCount = 1;

		// stencil info
		VkStencilOpState stencilOpState{};
		stencilOpState.failOp = failOp;
		stencilOpState.passOp = passOp;
		stencilOpState.depthFailOp = depthFailOp;
		stencilOpState.compareOp = compareOp;
		stencilOpState.compareMask = 0xFF;
		stencilOpState.writeMask = stencilWriteMask;
		stencilOpState.reference = 1;

		VkPipelineDepthStencilStateCreateInfo depthStencil{};
		depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		depthStencil.depthTestEnable = depthTestEnable;
		depthStencil.depthWriteEnable = depthWriteEnable;
		depthStencil.depthCompareOp = depthCompareOp;
		depthStencil.depthBoundsTestEnable = VK_FALSE;
		depthStencil.minDepthBounds = 0.f;
		depthStencil.maxDepthBounds = 1.f;
		depthStencil.stencilTestEnable = stencilTestEnable;
		depthStencil.front = stencilOpState;
		depthStencil.back = stencilOpState;

		std::vector<VkDynamicState> dynamicStates = {
			VK_DYNAMIC_STATE_VIEWPORT,
			VK_DYNAMIC_STATE_SCISSOR,
		};

		VkPipelineDynamicStateCreateInfo dynamicState{};
		dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
		dynamicState.pDynamicStates = dynamicStates.data();

		VkPipelineRasterizationStateCreateInfo rasterizer{};
		rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizer.depthClampEnable = VK_FALSE;
		rasterizer.rasterizerDiscardEnable = VK_FALSE;
		rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
		rasterizer.lineWidth = 1.f;
		rasterizer.cullMode = cullMode;
		rasterizer.frontFace = cullFace;
		rasterizer.depthBiasEnable = VK_FALSE;
		rasterizer.depthBiasConstantFactor = 0.f;
		rasterizer.depthBiasClamp = 0.f;
		rasterizer.depthBiasSlopeFactor = 0.f;

		VkPipelineMultisampleStateCreateInfo multisampling{};
		multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampling.sampleShadingEnable = VK_TRUE;
		multisampling.rasterizationSamples = msaaSamples;
		multisampling.minSampleShading = 0.2f;
		multisampling.pSampleMask = nullptr;
		multisampling.alphaToCoverageEnable = VK_FALSE;
		multisampling.alphaToOneEnable = VK_FALSE;

		VkPipelineColorBlendAttachmentState colorBlendAttachment{};
		colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		colorBlendAttachment.blendEnable = VK_TRUE;
		colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
		colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
		colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
		colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

		VkPipelineColorBlendStateCreateInfo colorBlending{};
		colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlending.logicOpEnable = VK_FALSE;
		colorBlending.logicOp = VK_LOGIC_OP_COPY;
		colorBlending.attachmentCount = 1;
		colorBlending.pAttachments = &colorBlendAttachment;
		colorBlending.blendConstants[0] = 0.f;
		colorBlending.blendConstants[1] = 0.f;
		colorBlending.blendConstants[2] = 0.f;
		colorBlending.blendConstants[3] = 0.f;

		VkGraphicsPipelineCreateInfo pipelineInfo{};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineInfo.stageCount = 2; // Two shader stages; vertex and fragment
		pipelineInfo.pStages = shaderStages;
		pipelineInfo.pVertexInputState = &vertexInputInfo;
		pipelineInfo.pInputAssemblyState = &inputAssembly;
		pipelineInfo.pViewportState = &viewportState;
		pipelineInfo.pRasterizationState = &rasterizer;
		pipelineInfo.pMultisampleState = &multisampling;
		pipelineInfo.pDepthStencilState = &depthStencil;
		pipelineInfo.pColorBlendState = &colorBlending;
		pipelineInfo.pDynamicState = &dynamicState;
		pipelineInfo.layout = pipelineLayout;
		pipelineInfo.renderPass = renderPass;
		pipelineInfo.subpass = 0;
		pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

		if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create primitive graphics pipeline!");
		}

		//END
		vkDestroyShaderModule(device, fragShaderModule, nullptr);
		vkDestroyShaderModule(device, vertShaderModule, nullptr);

		return Pipeline(device, pipeline, pipelineLayout);
	}
};

#endif

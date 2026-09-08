#pragma once 

#include "../../core/renderer/vulkanApp/vulkanApp.h"
#include <thread>

class ModelLoading : public IVulkanApp
{
	public:
	ModelLoading() = default;
	constexpr static int frames = 2;
	Model *  model;
	
	std::vector<VkPipelineShaderStageCreateInfo> shaderStages;	
	struct UniformData
	{
		alignas(16) glm::mat4 model;
		alignas(16) glm::mat4 view;
		alignas(16) glm::mat4 proj;
	};
	
	struct 
	{
		VkRenderPass renderPass;	
	} renderPasses{};
	
	VkPipelineLayout pipelineLayout{};
	struct
	{
		VkPipeline model;
	} pipelines;	

	VkDescriptorSetLayout descriptorSetLayout;
	struct DescriptorSets
	{
		std::vector<VkDescriptorSet> model;
	};

	std::array<DescriptorSets, frames> descriptorSets;

	struct UniformBuffers 
	{
		std::vector<VkBuffer> model;
	};
	
	struct UniformBuffersMapped
	{
		std::vector<void *> model;
	};
	
	struct UniformBuffersMemory
	{
		std::vector<VkDeviceMemory> model;
	};

	std::array<UniformBuffers, frames> uniformBuffers;	
	std::array<UniformBuffersMapped, frames> uniformBuffersMapped;	
	std::array<UniformBuffersMemory, frames> uniformBuffersMemory;	
	
	VkSampler sampler;

	struct
	{
		VkImageView imageView;
		VkImage     image;
		VkDeviceMemory imageMemory;
	} depth;		

	struct
	{
		VkImageView imageView;
		VkImage     image;
		VkDeviceMemory imageMemory;
	} resolved;		

	void loadModel()
	{
		model = new Model("/resource/models/backpack/backpack.obj", VulkanConfig::device, VulkanConfig::physicalDevice, VulkanConfig::graphicsAndComputeQueue);
//		model = new Model("/resource/models/Sponza-master/sponza.obj", "/resource/models/Sponza-master/", VulkanConfig::device, VulkanConfig::physicalDevice, VulkanConfig::graphicsAndComputeQueue);
	}

	void setUpRenderPass()
	{
		VkAttachmentDescription colorAttachment{
			.format = swapChainImageFormat,
			.samples = VulkanConfig::msaaSamples,
			.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
			.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
			.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
			.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
			.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
			.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
		};	
		
		VkAttachmentDescription colorAttachmentResolve{
		.format = swapChainImageFormat,
		.samples = VK_SAMPLE_COUNT_1_BIT,
		.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
		.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
		.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
		.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
		.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
		.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
		};

		VkAttachmentDescription depthAttachment{
			.format = VK_FORMAT_D32_SFLOAT_S8_UINT,
			.samples = VulkanConfig::msaaSamples,
			.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
			.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
			.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
			.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
			.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
			.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL};

		VkAttachmentReference colorAttachmentRef{
			.attachment = 0,
			.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};
	
		VkAttachmentReference depthAttachmentRef{
			.attachment = 1,
			.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL};

		VkAttachmentReference colorAttachmentResolveRef{
		.attachment = 2,
		.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};

		VkSubpassDescription subpass{
			.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
			.colorAttachmentCount = 1,
			.pColorAttachments = &colorAttachmentRef,
			.pResolveAttachments = &colorAttachmentResolveRef,
			.pDepthStencilAttachment = &depthAttachmentRef,
};
		
		VkSubpassDependency dependency{
			.srcSubpass = VK_SUBPASS_EXTERNAL,
			.dstSubpass = 0,
			.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
			.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
			.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
			.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT
		};	

		std::array<VkAttachmentDescription, 3> attachments{colorAttachment, depthAttachment, colorAttachmentResolve};
		
		VkRenderPassCreateInfo renderPassInfo{
			.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
			.attachmentCount = static_cast<uint32_t>(attachments.size()),
			.pAttachments = attachments.data(),
			.subpassCount = 1,
			.pSubpasses = &subpass,
			.dependencyCount = 1,
			.pDependencies = &dependency
		};
		
		if (vkCreateRenderPass(VulkanConfig::device, &renderPassInfo, nullptr, &renderPasses.renderPass) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create render pass!");
		};
	};
	
	void createFramebuffers()
	{
		swapChainFramebuffers.resize(swapChainImageViews.size());

		for (size_t i = 0; i < swapChainImageViews.size(); i++)
		{
			std::array<VkImageView, 3> attachments = { 
				resolved.imageView,
				depth.imageView,
				swapChainImageViews[i] 
			};
			
			VkFramebufferCreateInfo framebufferInfo{};
			framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			framebufferInfo.renderPass = renderPasses.renderPass;
			framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
			framebufferInfo.pAttachments = attachments.data();
			framebufferInfo.width = VulkanConfig::swapChainExtent.width;
			framebufferInfo.height = VulkanConfig::swapChainExtent.height;
			framebufferInfo.layers = 1;

			if (vkCreateFramebuffer(VulkanConfig::device, &framebufferInfo, nullptr, &swapChainFramebuffers[i]) != VK_SUCCESS)
			{
				throw std::runtime_error("failed to create framebuffer!");
			};
		}
	}

	void setupUniformBuffers()
	{
		VkDeviceSize bufferSize = sizeof(UniformData);	
		
		for (size_t j = 0; j < frames; j++)
		{
			for (size_t i = 0; i < model->meshes.size(); i++)
			{
				uniformBuffers[j].model.resize(model->meshes.size());
				uniformBuffersMapped[j].model.resize(model->meshes.size());
				uniformBuffersMemory[j].model.resize(model->meshes.size());

				Buffer::create(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, uniformBuffers[j].model[i], uniformBuffersMemory[j].model[i], VulkanConfig::device, VulkanConfig::physicalDevice);
				
				vkMapMemory(VulkanConfig::device, uniformBuffersMemory[j].model[i], 0, bufferSize, 0, &uniformBuffersMapped[j].model[i]);
			};
		};
	}

	uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
	{
		VkPhysicalDeviceMemoryProperties memProperties;
		vkGetPhysicalDeviceMemoryProperties(VulkanConfig::physicalDevice, &memProperties);

		for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
		{
			if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
			{
				return i;
			}
		}

		throw std::runtime_error("failed to find a suitable memory type!");
	}

	void setupResolved()
	{
		VkImageCreateInfo imageInfo{
			.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,	
			.flags = 0,
			.imageType = VK_IMAGE_TYPE_2D,
			.format = swapChainImageFormat,
			.mipLevels = 1,
			.arrayLayers = 1,
			.samples = VulkanConfig::msaaSamples,
			.tiling = VK_IMAGE_TILING_OPTIMAL,
			.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT,		 
			.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
			.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED	
		};
		
		imageInfo.extent.width = static_cast<uint32_t>(VulkanConfig::swapChainExtent.width);
		imageInfo.extent.height = static_cast<uint32_t>(VulkanConfig::swapChainExtent.height);
		imageInfo.extent.depth = 1;

		if(vkCreateImage(VulkanConfig::device, &imageInfo, nullptr, &resolved.image))
		{
			throw std::runtime_error("failed to create resolved image!");
		};	

		VkMemoryRequirements memRequirements;
		vkGetImageMemoryRequirements(VulkanConfig::device, resolved.image, &memRequirements);
		
		VkMemoryAllocateInfo allocInfo
		{
			.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
			.allocationSize = memRequirements.size,
			.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
		};

		if (vkAllocateMemory(VulkanConfig::device, &allocInfo, nullptr, &resolved.imageMemory) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to allocate memory for resolved image!");
		};
	
		vkBindImageMemory(VulkanConfig::device, resolved.image, resolved.imageMemory, 0);
		VkImageViewCreateInfo viewInfo{
			.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
			.image = resolved.image,
			.viewType = VK_IMAGE_VIEW_TYPE_2D,
			.format = swapChainImageFormat
		};			
		
		viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = 1;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = 1;
		
		if (vkCreateImageView(VulkanConfig::device, &viewInfo, nullptr, &resolved.imageView) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create resolved image view!");
		};
	};	

	void setupDepth()
	{
		VkImageCreateInfo imageInfo{
			.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,	
			.flags = 0,
			.imageType = VK_IMAGE_TYPE_2D,
			.format = VK_FORMAT_D32_SFLOAT_S8_UINT,
			.mipLevels = 1,
			.arrayLayers = 1,
			.samples = VulkanConfig::msaaSamples,
			.tiling = VK_IMAGE_TILING_OPTIMAL,
			.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,		 
			.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
			.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED	
		};
		
		imageInfo.extent.width = static_cast<uint32_t>(VulkanConfig::swapChainExtent.width);
		imageInfo.extent.height = static_cast<uint32_t>(VulkanConfig::swapChainExtent.height);
		imageInfo.extent.depth = 1;

		if(vkCreateImage(VulkanConfig::device, &imageInfo, nullptr, &depth.image))
		{
			throw std::runtime_error("failed to create image!");
		};	

		VkMemoryRequirements memRequirements;
		vkGetImageMemoryRequirements(VulkanConfig::device, depth.image, &memRequirements);
		
		VkMemoryAllocateInfo allocInfo
		{
			.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
			.allocationSize = memRequirements.size,
			.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
		};

		if (vkAllocateMemory(VulkanConfig::device, &allocInfo, nullptr, &depth.imageMemory) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to allocate memory for depth image!");
		};
	
		vkBindImageMemory(VulkanConfig::device, depth.image, depth.imageMemory, 0);
	
		VkCommandBuffer commandBuffer = CommandBuffer::beginSingleTimeCommands(VulkanConfig::device);		
		
		VkImageMemoryBarrier barrier{
			.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
			.srcAccessMask = 0,
			.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
			.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
			.newLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
			.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
			.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
			.image = depth.image	
		};

		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;	
		barrier.subresourceRange.baseMipLevel = 0;	
		barrier.subresourceRange.levelCount = 1;	
		barrier.subresourceRange.baseArrayLayer  = 0;	
		barrier.subresourceRange.layerCount = 1;	

		VkPipelineStageFlags srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;	
		VkPipelineStageFlags dstStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;		

		vkCmdPipelineBarrier(
			commandBuffer,
			srcStage, dstStage,
			0,
			0, nullptr,
			0, nullptr,
			1, &barrier);	

		CommandBuffer::endSingleTimeCommands(commandBuffer, VulkanConfig::graphicsAndComputeQueue, VulkanConfig::device);
	
		VkImageViewCreateInfo viewInfo{
			.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
			.image = depth.image,
			.viewType = VK_IMAGE_VIEW_TYPE_2D,
			.format = VK_FORMAT_D32_SFLOAT_S8_UINT,
		};			
		
		viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = 1;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = 1;
		
		if (vkCreateImageView(VulkanConfig::device, &viewInfo, nullptr, &depth.imageView) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create depth image view!");
		};
	};

	void setupSamplers()
	{
		VkPhysicalDeviceProperties properties{};
		vkGetPhysicalDeviceProperties(VulkanConfig::physicalDevice, &properties);
		
		VkSamplerCreateInfo samplerInfo{
			.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
			.magFilter = VK_FILTER_LINEAR,
			.minFilter = VK_FILTER_LINEAR,
			.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
			.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT,
			.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT,
			.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT,
			.mipLodBias = 0.f,
			.anisotropyEnable = VK_TRUE,
			.maxAnisotropy = properties.limits.maxSamplerAnisotropy,
			.compareEnable = VK_FALSE,
			.compareOp = VK_COMPARE_OP_ALWAYS,
			.minLod = 0.f,
			.maxLod = VK_LOD_CLAMP_NONE,
			.borderColor = VK_BORDER_COLOR_INT_OPAQUE_WHITE,
			.unnormalizedCoordinates = VK_FALSE
		};		
		
		if (vkCreateSampler(VulkanConfig::device, &samplerInfo, nullptr, &sampler))
		{
			throw std::runtime_error("failed to create sampler!");	
		};
	};

	void setupDescriptorSets()
	{
		VkDescriptorPool descriptorPool;
		VkDescriptorSetLayoutBinding vertexLayoutBinding{
			.binding = 0,
			.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			.descriptorCount = 1,
			.stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
			.pImmutableSamplers = nullptr
		};

		VkDescriptorSetLayoutBinding fragmentLayoutBinding{
			.binding = 1,
			.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			.descriptorCount = 1,
			.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
			.pImmutableSamplers = nullptr
		};

		std::array<VkDescriptorSetLayoutBinding, 2> setLayoutBindings{vertexLayoutBinding, fragmentLayoutBinding};
		
		VkDescriptorSetLayoutCreateInfo layoutInfo{
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
			.bindingCount = static_cast<uint32_t>(setLayoutBindings.size()),
			.pBindings = setLayoutBindings.data()
		};
	
		if (vkCreateDescriptorSetLayout(VulkanConfig::device, &layoutInfo, nullptr, &descriptorSetLayout))
		{
			throw std::runtime_error("Failed to create descriptor set layout!");
		};	
		
		std::array<VkDescriptorPoolSize, 2> poolSizes{};
		poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		poolSizes[0].descriptorCount = static_cast<uint32_t>(model->meshes.size() * frames) * 2;

		poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

		poolSizes[1].descriptorCount = static_cast<uint32_t>(model->meshes.size() * frames) * 2;

		VkDescriptorPoolCreateInfo poolInfo
		{
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
			.maxSets = static_cast<uint32_t>(model->meshes.size() * frames) * 2,
			.poolSizeCount = static_cast<uint32_t>(poolSizes.size()),
			.pPoolSizes = poolSizes.data()
		};	
		
		if (vkCreateDescriptorPool(VulkanConfig::device, &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create descriptor pool!");		
		}
			
		descriptorSets[0].model.resize(model->meshes.size());
		descriptorSets[1].model.resize(model->meshes.size());

		std::array<VkDescriptorSetLayout, frames> layouts{};
		layouts.fill(descriptorSetLayout);	
	
		VkDescriptorSetAllocateInfo allocInfo
		{
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
			.descriptorPool = descriptorPool,
			.descriptorSetCount = static_cast<uint32_t>(layouts.size()),
			.pSetLayouts = layouts.data()
		};	  

		for (size_t i = 0; i < model->meshes.size(); i++)
		{
			for (size_t j = 0; j < frames; j++)
			{
				if (vkAllocateDescriptorSets(VulkanConfig::device, &allocInfo, &descriptorSets[j].model[i]) != VK_SUCCESS)
				{
					throw std::runtime_error("failed to allocate descriptor sets!");
				};

				VkDescriptorBufferInfo bufferInfo{
					.buffer = uniformBuffers[j].model[i],
					.offset = 0,
					.range = sizeof(UniformData)
				};
				
				VkDescriptorImageInfo imageInfo
				{
					.sampler = sampler,
					.imageView = model->texture.imageView[i],
					.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
				};
				
				std::array<VkWriteDescriptorSet, 2> descriptorWrites{};
				
				descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				descriptorWrites[0].dstSet = descriptorSets[j].model[i];
				descriptorWrites[0].dstBinding = 0;
				descriptorWrites[0].dstArrayElement = 0;
				descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				descriptorWrites[0].descriptorCount = 1;
				descriptorWrites[0].pBufferInfo = &bufferInfo;
				
				descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				descriptorWrites[1].dstSet = descriptorSets[j].model[i];
				descriptorWrites[1].dstBinding = 1;
				descriptorWrites[1].dstArrayElement = 0;
				descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				descriptorWrites[1].descriptorCount = 1;
				descriptorWrites[1].pImageInfo = &imageInfo;
				
				vkUpdateDescriptorSets(VulkanConfig::device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(),0, nullptr);	
			};
		};
	}	

	void addShader(const std::string&& path, VkShaderStageFlagBits stage)
	{
		auto shaderCode = FileContext::readFile(path);

		VkShaderModule shaderModule = createShaderModule(shaderCode, VulkanConfig::device);
		VkPipelineShaderStageCreateInfo shaderStageInfo{};
		shaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shaderStageInfo.stage = stage;
		shaderStageInfo.module = shaderModule;
		shaderStageInfo.pName = "main";
		
		assert(stage != 0);
		
		shaderStages.push_back(shaderStageInfo);	
	}

	void setupPipelineLayout()
	{
		VkPipelineLayoutCreateInfo pipelineLayoutInfo{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
			.setLayoutCount = 1,
			.pSetLayouts = &descriptorSetLayout,
			.pushConstantRangeCount = 0,
			.pPushConstantRanges = nullptr
		};
	
		if (vkCreatePipelineLayout(VulkanConfig::device, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create pipeline layout!");
		};
	};

	void setupPipelines()
	{
		setupPipelineLayout();
		
		addShader(SHADER_DIRECTORY + "/textureMapping/vert.spv", VK_SHADER_STAGE_VERTEX_BIT);	
		addShader(SHADER_DIRECTORY + "/textureMapping/frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);	
	
		VkVertexInputBindingDescription bindingDescription{
			.binding = 0,
			.stride = sizeof(Vertex),
			.inputRate = VK_VERTEX_INPUT_RATE_VERTEX
		};		
		
		std::array<VkVertexInputBindingDescription, 1> bindingDescriptions{bindingDescription};
		
		std::array<VkVertexInputAttributeDescription, 4> attributeDescriptions;
		
		attributeDescriptions[0].binding = 0;
		attributeDescriptions[0].location = 0;
		attributeDescriptions[0].format = VK_FORMAT_R32G32B32A32_SFLOAT;
		attributeDescriptions[0].offset = offsetof(Vertex, pos);
		attributeDescriptions[1].binding = 0;
		attributeDescriptions[1].location = 1;
		attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[1].offset = offsetof(Vertex, color);
		attributeDescriptions[2].binding = 0;
		attributeDescriptions[2].location = 2;
		attributeDescriptions[2].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[2].offset = offsetof(Vertex, normal);
		attributeDescriptions[3].binding = 0;
		attributeDescriptions[3].location = 3;
		attributeDescriptions[3].format = VK_FORMAT_R32G32_SFLOAT;
		attributeDescriptions[3].offset = offsetof(Vertex, texCoord);
		
		VkPipelineVertexInputStateCreateInfo vertexInputInfo{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
			.vertexBindingDescriptionCount = static_cast<uint32_t>(bindingDescriptions.size()),	
			.pVertexBindingDescriptions = bindingDescriptions.data(),
			.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size()),
			.pVertexAttributeDescriptions = attributeDescriptions.data()
		};
		
		VkPipelineInputAssemblyStateCreateInfo inputAssembly{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
			.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
			.primitiveRestartEnable = VK_FALSE
		};

		VkViewport viewport{
			.x = 0.f,
			.y = 0.f,
			.width = (float)VulkanConfig::swapChainExtent.width,
			.height = (float)VulkanConfig::swapChainExtent.height,
			.minDepth = 0.f,
			.maxDepth = 1.f};

		VkPipelineViewportStateCreateInfo viewportState{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
			.viewportCount = 1,
			.scissorCount = 1};

		// stencil info
		VkStencilOpState stencilOpState {
			.failOp = VK_STENCIL_OP_KEEP,
			.passOp = VK_STENCIL_OP_REPLACE,
			.depthFailOp = VK_STENCIL_OP_KEEP,
			.compareOp = VK_COMPARE_OP_ALWAYS,
			.compareMask = 0xFF,
			.writeMask = 0xFF,
			.reference = 1 
		};

		VkPipelineDepthStencilStateCreateInfo depthStencil {
			.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
			.depthTestEnable = VK_TRUE,
			.depthWriteEnable = VK_TRUE,
			.depthCompareOp = VK_COMPARE_OP_LESS,
			.depthBoundsTestEnable = VK_FALSE,
			.stencilTestEnable = VK_FALSE,
			//.front = stencilOpState,
			//.back = stencilOpState,
			.minDepthBounds = 0.f,
			.maxDepthBounds = 1.f
		};

		std::vector<VkDynamicState> dynamicStates = {
			VK_DYNAMIC_STATE_VIEWPORT,
			VK_DYNAMIC_STATE_SCISSOR,
		};

		VkPipelineDynamicStateCreateInfo dynamicState {
			.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
			.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size()),
			.pDynamicStates = dynamicStates.data()
		};

		VkPipelineRasterizationStateCreateInfo rasterizer{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
			.depthClampEnable = VK_FALSE,
			.rasterizerDiscardEnable = VK_FALSE,
			.polygonMode = VK_POLYGON_MODE_FILL,
			.cullMode = VK_CULL_MODE_NONE,
			.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
			.depthBiasEnable = VK_FALSE,
			.depthBiasConstantFactor = 0.f,
			.depthBiasClamp = 0.f,
			.depthBiasSlopeFactor = 0.f,
			.lineWidth = 1.f
		};

		VkPipelineMultisampleStateCreateInfo multisampling{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
			.rasterizationSamples = VulkanConfig::msaaSamples,
			.minSampleShading = 0.2f,
			.pSampleMask = nullptr,
			.alphaToCoverageEnable = VK_FALSE,
			.alphaToOneEnable = VK_FALSE
		};

		VkPipelineColorBlendAttachmentState colorBlendAttachment{
			.blendEnable = VK_TRUE,
			.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA,
			.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
			.colorBlendOp = VK_BLEND_OP_ADD,
			.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
			.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
			.alphaBlendOp = VK_BLEND_OP_ADD,
			.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT
		};

		VkPipelineColorBlendStateCreateInfo colorBlending{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
			.logicOpEnable = VK_FALSE,
			.logicOp = VK_LOGIC_OP_COPY,
			.attachmentCount = 1,
			.pAttachments = &colorBlendAttachment,
			.blendConstants = {0.f, 0.f, 0.f, 0.f}
		};

		VkGraphicsPipelineCreateInfo pipelineInfo{
			.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
			.stageCount = static_cast<uint32_t>(shaderStages.size()),
			.pStages = shaderStages.data(),
			.pVertexInputState = &vertexInputInfo,
			.pInputAssemblyState = &inputAssembly,
			.pViewportState = &viewportState,
			.pRasterizationState = &rasterizer,
			.pMultisampleState = &multisampling,
			.pDepthStencilState = &depthStencil,
			.pColorBlendState = &colorBlending,
			.pDynamicState = &dynamicState,
			.layout = pipelineLayout,
			.renderPass = renderPasses.renderPass,
			.subpass = 0,
			.basePipelineHandle = VK_NULL_HANDLE
		};

		if (vkCreateGraphicsPipelines(VulkanConfig::device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipelines.model) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create primitive graphics pipeline!");
		}
	};
	
	void updateUniformBuffer(uint32_t currentImage)
	{
		for (size_t i = 0; i < model->meshes.size(); i++)
		{
			UniformData uniformData;
			
			uniformData.model = glm::mat4(1.);		
			uniformData.view = camera.getViewMatrix();
			uniformData.proj = glm::perspective(glm::radians(45.f), VulkanConfig::swapChainExtent.width / (float)VulkanConfig::swapChainExtent.height, 0.1f, FAR_PLANE);
			uniformData.proj[1][1] *= -1.;
			
			memcpy(uniformBuffersMapped[currentImage].model[i], &uniformData, sizeof(uniformData));
		};
	};

	void processInput(GLFWwindow * window)
	{
		camera.cameraSpeed = 10.f * lastFrameTime;
		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		{
			camera.move(FORWARD);
		}
		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
			camera.move(BACKWARD);
		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
			camera.move(LEFT);
		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
			camera.move(RIGHT);
	};
	
	void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex)
	{
		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = 0;
		beginInfo.pInheritanceInfo = nullptr;

		if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to begin recording command buffer!");
		}

		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = renderPasses.renderPass;
		renderPassInfo.framebuffer = swapChainFramebuffers[imageIndex];
		renderPassInfo.renderArea.offset = { 0,0 };
		renderPassInfo.renderArea.extent = VulkanConfig::swapChainExtent;

		std::array<VkClearValue, 2> clearValues{};
		clearValues[0].color = {{1.f, 1.f, 1.f, 1.f}};
		clearValues[1].depthStencil = {1.f, 0};

		renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
		renderPassInfo.pClearValues = clearValues.data();

		vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		VkDeviceSize offsets[] = { 0 };

		VkViewport viewport{};
		viewport.x = 0.f;
		viewport.y = 0.f;
		viewport.width = static_cast<float>(VulkanConfig::swapChainExtent.width);
		viewport.height = static_cast<float>(VulkanConfig::swapChainExtent.height);
		viewport.minDepth = 0.f;
		viewport.maxDepth = 1.f;

		VkRect2D scissor{};
		scissor.offset = {0, 0};
		scissor.extent = VulkanConfig::swapChainExtent;
		vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

		clearValues[0].color = {{.1f, .1f, .1f, 1.f}};
		clearValues[1].depthStencil = {1.0f, 0};
		renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
		renderPassInfo.pClearValues = clearValues.data();

		vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelines.model);	
		
		for (size_t i = 0; i < model->meshes.size(); i++)
		{
			const Mesh mesh = model->meshes[i];

			vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSets[currentFrame].model[i], 0, nullptr);
			
			vkCmdBindVertexBuffers(commandBuffer, 0, 1, &model->buffer.buffers[i], offsets);
			vkCmdBindIndexBuffer(commandBuffer, model->buffer.index[i], 0, VK_INDEX_TYPE_UINT32);

			vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(mesh.indices.size()), 1, 0, 0, 0);
		};
	
//		vkCmdBindIndexBuffer(commandBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT32);		

//		vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(ModelLoading::quadIndices.size()), 1, 0, 0, 0);

		vkCmdEndRenderPass(commandBuffer);

		if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to record command buffer!");
		}
	};

	void init(GLFWwindow* window)
	{
		IVulkanApp::init(window);	
		setUpRenderPass();
		setupSamplers();
		double startTime = glfwGetTime();
		loadModel();
		std::cout << "Finished in " << glfwGetTime() - startTime << " seconds\n";
		setupResolved();
		setupDepth();
		setupUniformBuffers();
		setupDescriptorSets();
		setupPipelines();
		createFramebuffers();
	};

	void cleanup(GLFWwindow * window)
	{
		cleanupSwapChain();

		if (enableValidationLayers)
		{
			DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
		}

		for (size_t j = 0; j < model->meshes.size(); j++)
		{
			for (size_t i = 0; i < frames; i++)
			{
				vkDestroyBuffer(VulkanConfig::device, uniformBuffers[i].model[j], nullptr);
				vkFreeMemory(VulkanConfig::device, uniformBuffersMemory[i].model[j], nullptr);
			}		
		};

		vkDestroyDescriptorSetLayout(VulkanConfig::device, descriptorSetLayout, nullptr);

		vkDestroyBuffer(VulkanConfig::device, indexBuffer, nullptr);
		vkFreeMemory(VulkanConfig::device, indexBufferMemory, nullptr);
		
		vkDestroyBuffer(VulkanConfig::device, vertexCubeBuffer, nullptr);
		vkFreeMemory(VulkanConfig::device, vertexCubeBufferMemory, nullptr);
		vkDestroyBuffer(VulkanConfig::device, vertexCubeBuffer, nullptr);

		vkFreeMemory(VulkanConfig::device, vertexCubeBufferMemory, nullptr);

		for (size_t i = 0; i < model->meshes.size(); i++)
		{
			vkDestroyImage(VulkanConfig::device, model->texture.image[i], nullptr);
			vkFreeMemory(VulkanConfig::device, model->texture.imageMemory[i], nullptr);
		};

		vkDestroyRenderPass(VulkanConfig::device, renderPasses.renderPass, nullptr);

		vkDestroyDevice(VulkanConfig::device, nullptr);

		vkDestroySurfaceKHR(instance, surface, nullptr);
		
		vkDestroyInstance(instance, nullptr);

		glfwDestroyWindow(window);

		glfwTerminate();
	};

	void recreateSwapChain(GLFWwindow * window)
	{
		int width = 0, height = 0;
		glfwGetFramebufferSize(window, &width, &height);

		while (width == 0 || height == 0)
		{
			glfwGetFramebufferSize(window, &width, &height);
			glfwWaitEvents();
		}

		vkDeviceWaitIdle(VulkanConfig::device);

		cleanupSwapChain();

		createSwapChain(window);
		createImageViews();
		setupResolved();
		setupDepth();

		createFramebuffers();
	}

};

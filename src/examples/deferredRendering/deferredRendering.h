#pragma once 

#include "../../core/renderer/vulkanApp/vulkanApp.h"

class DeferredRendering : public IVulkanApp
{
	public:
	DeferredRendering() = default;
	constexpr static int frames = 2;
	constexpr static int LIGHT_COUNT = 5;
	Model * model;

	std::array<glm::vec3, LIGHT_COUNT> lightColors
	{
		glm::vec3(200., 0., 0.),
		glm::vec3(0., 200., 0.),
		glm::vec3(200., 0., 200.),
		glm::vec3(0., 0., 200.),
		glm::vec3(200., 200., 0.)
	};

	glm::vec3 lightPositions[LIGHT_COUNT] =
	{
		glm::vec3(0., 0., 49.5),
		glm::vec3(0., 4., 25.),
		glm::vec3(2., 0., 10.),
		glm::vec3(0., 0., 1.),
		glm::vec3(1., 1., 39.)
	};

	std::vector<VkPipelineShaderStageCreateInfo> shaderStages;	
	std::vector<VkFramebuffer> offscreenFramebuffers;

	struct LightUniform
	{
		alignas(16) glm::mat4 model;
		alignas(16) glm::mat4 view;
		alignas(16) glm::mat4 proj;
		alignas(16) glm::vec3 lightColor;
	};

	struct ObjectUniform
	{
		alignas(16) glm::mat4 model;
		alignas(16) glm::mat4 view;
		alignas(16) glm::mat4 proj;
		alignas(16) glm::vec3 cameraPos;
		alignas(16) glm::vec3 lightColor[LIGHT_COUNT];
		alignas(16) glm::vec3 lightPos[LIGHT_COUNT];
	};

	struct 
	{
		VkRenderPass renderPass;	
		VkRenderPass offscreenPass;	
	} renderPasses{};
	
	struct
	{
		VkPipelineLayout cube;		
		VkPipelineLayout quad;		
		VkPipelineLayout light;		
		VkPipelineLayout model;		
	} pipelineLayouts;
	
	struct
	{
		VkPipeline cube;
		VkPipeline quad;
		VkPipeline light;
		VkPipeline model;
	} pipelines;	

	struct
	{
		VkDescriptorSetLayout cube;
		VkDescriptorSetLayout quad;
		VkDescriptorSetLayout light;
	} descriptorSetLayouts;

	struct DescriptorSets
	{
		std::vector<VkDescriptorSet> light;
		VkDescriptorSet cube;
		VkDescriptorSet quad;
	};

	std::array<DescriptorSets, frames> descriptorSets;

	struct UniformBuffers 
	{
		VkBuffer cube;
		std::vector<VkBuffer> light;
	};
	
	struct UniformBuffersMapped
	{
		void * cube;
		std::vector<void *> light;
	};
	
	struct UniformBuffersMemory
	{
		VkDeviceMemory cube;
		std::vector<VkDeviceMemory> light;
	};

	std::array<UniformBuffers, frames> uniformBuffers;	
	std::array<UniformBuffersMapped, frames> uniformBuffersMapped;	
	std::array<UniformBuffersMemory, frames> uniformBuffersMemory;	
	VkSampler sampler;
	struct Texture
	{
		VkImageView imageView;
		VkImage     image;
		VkDeviceMemory imageMemory;
	};	

	struct
	{
		Texture wood;
		Texture position;
		Texture normal;
		Texture albedo;
		Texture specular;
		Texture depth;
	} textures;

	void setupModelDescriptorSets()
	{
		model->setupDescriptorSets<ObjectUniform>();	
	}	

	void loadModel()
	{
//		model = new Model("/resource/models/backpack/backpack.obj", VulkanConfig::device, VulkanConfig::physicalDevice, VulkanConfig::graphicsAndComputeQueue);
		model = new Model("/resource/models/test/backpack.fbx", VulkanConfig::device, VulkanConfig::physicalDevice, VulkanConfig::graphicsAndComputeQueue);
//		model = new Model("/resource/models/Sponza-master/sponza.obj", VulkanConfig::device, VulkanConfig::physicalDevice, VulkanConfig::graphicsAndComputeQueue);
		model->setupUniforms<ObjectUniform>();
	}

	void setupRenderPass()
	{
		VkAttachmentDescription colorAttachmentResolve{
		.format = swapChainImageFormat,
		.samples = VK_SAMPLE_COUNT_1_BIT,
		.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
		.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
		.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
		.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
		.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
		.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
		};

		VkAttachmentReference colorAttachmentRef{
			.attachment = 0,
			.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};

		VkSubpassDescription subpass{
			.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
			.colorAttachmentCount = 1,
			.pColorAttachments = &colorAttachmentRef
		};	

		VkSubpassDependency dependency{};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
		dependency.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

		std::array<VkAttachmentDescription, 1> attachments = {colorAttachmentResolve};

		VkRenderPassCreateInfo renderPassInfo{
			.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
			.attachmentCount = static_cast<uint32_t>(attachments.size()),	
			.pAttachments = attachments.data(),
			.subpassCount = 1,
			.pSubpasses = &subpass,
			.dependencyCount = 1,
			.pDependencies = &dependency
		};
		
		if (vkCreateRenderPass(VulkanConfig::device, &renderPassInfo, nullptr, &renderPasses.renderPass))
		{
			throw std::runtime_error("Failed to create render pass!");
		};
	};

	void setupOffscreenPass()
	{
		VkAttachmentDescription highResAttachment{
			.format = VK_FORMAT_R16G16B16A16_SFLOAT,
			.samples = VK_SAMPLE_COUNT_1_BIT,
			.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
			.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
			.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
			.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
			.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
			.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
		};

		VkAttachmentDescription lowResAttachment{
			.format = VK_FORMAT_R8G8B8A8_SRGB,
			.samples = VK_SAMPLE_COUNT_1_BIT,
			.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
			.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
			.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
			.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
			.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
			.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
		};

		VkAttachmentDescription depthAttachment{
			.format = VK_FORMAT_D32_SFLOAT_S8_UINT,
			.samples = VK_SAMPLE_COUNT_1_BIT,
			.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
			.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
			.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
			.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
			.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
			.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL};

		VkAttachmentReference depthAttachmentRef{
			.attachment = 0,
			.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL};

		VkAttachmentReference positionAttachmentRef{
		.attachment = 1,
		.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};

		VkAttachmentReference normalAttachmentRef{
		.attachment = 2,
		.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};
		VkAttachmentReference albedoAttachmentRef{
		.attachment = 3,
		.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};

		VkAttachmentReference specularAttachmentRef{
		.attachment = 4,
		.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};

		std::array<VkAttachmentReference, 4> colorAttachments{positionAttachmentRef, normalAttachmentRef, albedoAttachmentRef, specularAttachmentRef};	

		VkSubpassDescription subpass{
			.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
			.colorAttachmentCount = 4,
			.pColorAttachments = colorAttachments.data(),
			.pDepthStencilAttachment = &depthAttachmentRef,
};
		
		std::array<VkSubpassDependency, 2> dependency{};
		
		dependency[0].srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency[0].dstSubpass = 0;
		dependency[0].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
		dependency[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		dependency[0].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		dependency[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

		dependency[1].srcSubpass = 0;
		dependency[1].dstSubpass = VK_SUBPASS_EXTERNAL;
		dependency[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		dependency[1].dstStageMask =  VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		dependency[1].dstAccessMask =  VK_ACCESS_SHADER_READ_BIT;

		std::array<VkAttachmentDescription, 5> attachments{depthAttachment, highResAttachment, highResAttachment, lowResAttachment, lowResAttachment};
		
		VkRenderPassCreateInfo renderPassInfo{
			.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
			.attachmentCount = static_cast<uint32_t>(attachments.size()),
			.pAttachments = attachments.data(),
			.subpassCount = 1,
			.pSubpasses = &subpass,
			.dependencyCount = 1,
			.pDependencies = dependency.data()
		};
		
		if (vkCreateRenderPass(VulkanConfig::device, &renderPassInfo, nullptr, &renderPasses.offscreenPass) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create render pass!");
		};
	};
		
	void createSwapChainFramebuffers()
	{
		swapChainFramebuffers.resize(swapChainImageViews.size());

		for (size_t i = 0; i < swapChainImageViews.size(); i++)
		{
			std::array<VkImageView, 1> attachments = { 
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

	void createOffscreenFramebuffers()
	{
		offscreenFramebuffers.resize(swapChainImageViews.size());

		for (size_t i = 0; i < swapChainImageViews.size(); i++)
		{
			std::array<VkImageView, 5> attachments = { 
				textures.depth.imageView,
				textures.position.imageView,
				textures.normal.imageView,
				textures.albedo.imageView,
				textures.specular.imageView,
			};
			
			VkFramebufferCreateInfo framebufferInfo{};
			framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			framebufferInfo.renderPass = renderPasses.offscreenPass;
			framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
			framebufferInfo.pAttachments = attachments.data();
			framebufferInfo.width = VulkanConfig::swapChainExtent.width;
			framebufferInfo.height = VulkanConfig::swapChainExtent.height;
			framebufferInfo.layers = 1;

			if (vkCreateFramebuffer(VulkanConfig::device, &framebufferInfo, nullptr, &offscreenFramebuffers[i]) != VK_SUCCESS)
			{
				throw std::runtime_error("failed to create framebuffer!");
			};
		}
	}

	void setupUniformBuffers()
	{
		VkDeviceSize objectBufferSize = sizeof(ObjectUniform);	
		VkDeviceSize lightBufferSize = sizeof(LightUniform);	

		for (size_t i = 0; i < frames; i++)
		{
			Buffer::create(objectBufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, uniformBuffers[i].cube, uniformBuffersMemory[i].cube, VulkanConfig::device, VulkanConfig::physicalDevice);
			
			vkMapMemory(VulkanConfig::device, uniformBuffersMemory[i].cube, 0, objectBufferSize, 0, &uniformBuffersMapped[i].cube);
		};
		
		//Light
		uniformBuffers[0].light.resize(LIGHT_COUNT);
		uniformBuffers[1].light.resize(LIGHT_COUNT);
		uniformBuffersMemory[0].light.resize(LIGHT_COUNT);
		uniformBuffersMemory[1].light.resize(LIGHT_COUNT);
		uniformBuffersMapped[0].light.resize(LIGHT_COUNT);
		uniformBuffersMapped[1].light.resize(LIGHT_COUNT);

		for (size_t j = 0; j < LIGHT_COUNT; j++)
		{
			for (size_t i = 0; i < frames; i++)
			{
				Buffer::create(lightBufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, uniformBuffers[i].light[j], uniformBuffersMemory[i].light[j], VulkanConfig::device, VulkanConfig::physicalDevice);
				
				vkMapMemory(VulkanConfig::device, uniformBuffersMemory[i].light[j], 0, lightBufferSize, 0, &uniformBuffersMapped[i].light[j]);
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

	void setupAlbedo()
	{
		VkImageCreateInfo imageInfo{
			.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,	
			.flags = 0,
			.imageType = VK_IMAGE_TYPE_2D,
			.format = VK_FORMAT_R8G8B8A8_SRGB,
			.mipLevels = 1,
			.arrayLayers = 1,
			.samples = VK_SAMPLE_COUNT_1_BIT,
			.tiling = VK_IMAGE_TILING_OPTIMAL,
			.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,		 
			.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
			.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED	
		};
		
		imageInfo.extent.width = static_cast<uint32_t>(VulkanConfig::swapChainExtent.width);
		imageInfo.extent.height = static_cast<uint32_t>(VulkanConfig::swapChainExtent.height);
		imageInfo.extent.depth = 1;

		if(vkCreateImage(VulkanConfig::device, &imageInfo, nullptr, &textures.albedo.image))
		{
			throw std::runtime_error("failed to create albedo image!");
		};	

		VkMemoryRequirements memRequirements;
		vkGetImageMemoryRequirements(VulkanConfig::device, textures.albedo.image, &memRequirements);
		
		VkMemoryAllocateInfo allocInfo
		{
			.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
			.allocationSize = memRequirements.size,
			.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
		};

		if (vkAllocateMemory(VulkanConfig::device, &allocInfo, nullptr, &textures.albedo.imageMemory) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to allocate memory for albedo image!");
		};
	
		vkBindImageMemory(VulkanConfig::device, textures.albedo.image, textures.albedo.imageMemory, 0);
		VkImageViewCreateInfo viewInfo{
			.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
			.image = textures.albedo.image,
			.viewType = VK_IMAGE_VIEW_TYPE_2D,
			.format = VK_FORMAT_R8G8B8A8_SRGB
		};			
		
		viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = 1;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = 1;
		
		if (vkCreateImageView(VulkanConfig::device, &viewInfo, nullptr, &textures.albedo.imageView) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create color image view!");
		};
	};	

	void setupSpecular()
	{
		VkImageCreateInfo imageInfo{
			.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,	
			.flags = 0,
			.imageType = VK_IMAGE_TYPE_2D,
			.format = VK_FORMAT_R8G8B8A8_SRGB,
			.mipLevels = 1,
			.arrayLayers = 1,
			.samples = VK_SAMPLE_COUNT_1_BIT,
			.tiling = VK_IMAGE_TILING_OPTIMAL,
			.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,		 
			.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
			.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED	
		};
		
		imageInfo.extent.width = static_cast<uint32_t>(VulkanConfig::swapChainExtent.width);
		imageInfo.extent.height = static_cast<uint32_t>(VulkanConfig::swapChainExtent.height);
		imageInfo.extent.depth = 1;

		if(vkCreateImage(VulkanConfig::device, &imageInfo, nullptr, &textures.specular.image))
		{
			throw std::runtime_error("failed to create resolved image!");
		};	

		VkMemoryRequirements memRequirements;
		vkGetImageMemoryRequirements(VulkanConfig::device, textures.specular.image, &memRequirements);
		
		VkMemoryAllocateInfo allocInfo
		{
			.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
			.allocationSize = memRequirements.size,
			.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
		};

		if (vkAllocateMemory(VulkanConfig::device, &allocInfo, nullptr, &textures.specular.imageMemory) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to allocate memory for specular image!");
		};
	
		vkBindImageMemory(VulkanConfig::device, textures.specular.image, textures.specular.imageMemory, 0);
		VkImageViewCreateInfo viewInfo{
			.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
			.image = textures.specular.image,
			.viewType = VK_IMAGE_VIEW_TYPE_2D,
			.format = VK_FORMAT_R8G8B8A8_SRGB
		};			
		
		viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = 1;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = 1;
		
		if (vkCreateImageView(VulkanConfig::device, &viewInfo, nullptr, &textures.specular.imageView) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create color image view!");
		};
	};	

	void setupNormal()
	{
		VkImageCreateInfo imageInfo{
			.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,	
			.flags = 0,
			.imageType = VK_IMAGE_TYPE_2D,
			.format = VK_FORMAT_R16G16B16A16_SFLOAT,
			.mipLevels = 1,
			.arrayLayers = 1,
			.samples = VK_SAMPLE_COUNT_1_BIT,
			.tiling = VK_IMAGE_TILING_OPTIMAL,
			.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,		 
			.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
			.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED	
		};
		
		imageInfo.extent.width = static_cast<uint32_t>(VulkanConfig::swapChainExtent.width);
		imageInfo.extent.height = static_cast<uint32_t>(VulkanConfig::swapChainExtent.height);
		imageInfo.extent.depth = 1;

		if(vkCreateImage(VulkanConfig::device, &imageInfo, nullptr, &textures.normal.image))
		{
			throw std::runtime_error("failed to create resolved image!");
		};	

		VkMemoryRequirements memRequirements;
		vkGetImageMemoryRequirements(VulkanConfig::device, textures.normal.image, &memRequirements);
		
		VkMemoryAllocateInfo allocInfo
		{
			.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
			.allocationSize = memRequirements.size,
			.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
		};

		if (vkAllocateMemory(VulkanConfig::device, &allocInfo, nullptr, &textures.normal.imageMemory) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to allocate memory for normal image!");
		};
	
		vkBindImageMemory(VulkanConfig::device, textures.normal.image, textures.normal.imageMemory, 0);
		VkImageViewCreateInfo viewInfo{
			.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
			.image = textures.normal.image,
			.viewType = VK_IMAGE_VIEW_TYPE_2D,
			.format = VK_FORMAT_R16G16B16A16_SFLOAT
		};			
		
		viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = 1;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = 1;
		
		if (vkCreateImageView(VulkanConfig::device, &viewInfo, nullptr, &textures.normal.imageView) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create color image view!");
		};
	};	

	void setupPosition()
	{
		VkImageCreateInfo imageInfo{
			.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,	
			.flags = 0,
			.imageType = VK_IMAGE_TYPE_2D,
			.format = VK_FORMAT_R16G16B16A16_SFLOAT,
			.mipLevels = 1,
			.arrayLayers = 1,
			.samples = VK_SAMPLE_COUNT_1_BIT,
			.tiling = VK_IMAGE_TILING_OPTIMAL,
			.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,		 
			.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
			.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED	
		};
		
		imageInfo.extent.width = static_cast<uint32_t>(VulkanConfig::swapChainExtent.width);
		imageInfo.extent.height = static_cast<uint32_t>(VulkanConfig::swapChainExtent.height);
		imageInfo.extent.depth = 1;

		if(vkCreateImage(VulkanConfig::device, &imageInfo, nullptr, &textures.position.image))
		{
			throw std::runtime_error("failed to create resolved image!");
		};	

		VkMemoryRequirements memRequirements;
		vkGetImageMemoryRequirements(VulkanConfig::device, textures.position.image, &memRequirements);
		
		VkMemoryAllocateInfo allocInfo
		{
			.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
			.allocationSize = memRequirements.size,
			.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
		};

		if (vkAllocateMemory(VulkanConfig::device, &allocInfo, nullptr, &textures.position.imageMemory) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to allocate memory for position image!");
		};
	
		vkBindImageMemory(VulkanConfig::device, textures.position.image, textures.position.imageMemory, 0);
		VkImageViewCreateInfo viewInfo{
			.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
			.image = textures.position.image,
			.viewType = VK_IMAGE_VIEW_TYPE_2D,
			.format = VK_FORMAT_R16G16B16A16_SFLOAT
		};			
		
		viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = 1;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = 1;
		
		if (vkCreateImageView(VulkanConfig::device, &viewInfo, nullptr, &textures.position.imageView) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create color image view!");
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
			.samples = VK_SAMPLE_COUNT_1_BIT, 
			.tiling = VK_IMAGE_TILING_OPTIMAL,
			.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,		 
			.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
			.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED	
		};
		
		imageInfo.extent.width = static_cast<uint32_t>(VulkanConfig::swapChainExtent.width);
		imageInfo.extent.height = static_cast<uint32_t>(VulkanConfig::swapChainExtent.height);
		imageInfo.extent.depth = 1;

		if(vkCreateImage(VulkanConfig::device, &imageInfo, nullptr, &textures.depth.image))
		{
			throw std::runtime_error("failed to create image!");
		};	

		VkMemoryRequirements memRequirements;
		vkGetImageMemoryRequirements(VulkanConfig::device, textures.depth.image, &memRequirements);
		
		VkMemoryAllocateInfo allocInfo
		{
			.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
			.allocationSize = memRequirements.size,
			.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
		};

		if (vkAllocateMemory(VulkanConfig::device, &allocInfo, nullptr, &textures.depth.imageMemory) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to allocate memory for depth image!");
		};
	
		vkBindImageMemory(VulkanConfig::device, textures.depth.image, textures.depth.imageMemory, 0);
	
		VkCommandBuffer commandBuffer = CommandBuffer::beginSingleTimeCommands(VulkanConfig::device);		
		
		VkImageMemoryBarrier barrier{
			.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
			.srcAccessMask = 0,
			.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
			.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
			.newLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
			.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
			.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
			.image = textures.depth.image	
		};

		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT| VK_IMAGE_ASPECT_STENCIL_BIT;	
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
			.image = textures.depth.image,
			.viewType = VK_IMAGE_VIEW_TYPE_2D,
			.format = VK_FORMAT_D32_SFLOAT_S8_UINT,
		};			
		
		viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = 1;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = 1;
		
		if (vkCreateImageView(VulkanConfig::device, &viewInfo, nullptr, &textures.depth.imageView) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create depth image view!");
		};
	};

	void loadTexture()
	{
		const std::string path = ROOT_DIR + "/resource/textures/wood.png";
		int texWidth, texHeight, texChannels;
		stbi_uc* pixels = stbi_load(path.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);		
		
		if (!pixels)
		{
			throw std::runtime_error("failed to load texture image! Path: " + path);	
		};
		
		VkDeviceSize imageSize = texWidth * texHeight * 4;
		int mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(texWidth, texHeight)))) + 1;	
	
		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;
		
		Buffer::create(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT|VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory, VulkanConfig::device, VulkanConfig::physicalDevice);		
		
		void * data;
		vkMapMemory(VulkanConfig::device, stagingBufferMemory, 0, imageSize, 0, &data);
		
		memcpy(data, pixels, static_cast<size_t>(imageSize));	
		vkUnmapMemory(VulkanConfig::device, stagingBufferMemory);
		
		stbi_image_free(pixels);
		
		VkImageCreateInfo imageInfo{
			.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
			.flags = 0,
			.imageType = VK_IMAGE_TYPE_2D,
			.format = VK_FORMAT_R8G8B8A8_SRGB,
			.mipLevels = static_cast<uint32_t>(mipLevels),
			.arrayLayers = 1,
			.samples = VK_SAMPLE_COUNT_1_BIT,
			.tiling = VK_IMAGE_TILING_OPTIMAL,
			.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,		 
			.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
			.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED};

		imageInfo.extent.width = static_cast<uint32_t>(texWidth);
		imageInfo.extent.height = static_cast<uint32_t>(texHeight);
		imageInfo.extent.depth = 1;

		if(vkCreateImage(VulkanConfig::device, &imageInfo, nullptr, &textures.wood.image))
		{
			throw std::runtime_error("failed to create image!");
		};	
		
		VkMemoryRequirements memRequirements;
		vkGetImageMemoryRequirements(VulkanConfig::device, textures.wood.image, &memRequirements);
		
		VkMemoryAllocateInfo allocInfo{
			.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
			.allocationSize = memRequirements.size,
			.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
		};
		
		if (vkAllocateMemory(VulkanConfig::device, &allocInfo, nullptr, &textures.wood.imageMemory) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to allocate image memory!");	
		};
		
		vkBindImageMemory(VulkanConfig::device, textures.wood.image, textures.wood.imageMemory, 0);
		
		VkCommandBuffer commandBuffer = CommandBuffer::beginSingleTimeCommands(VulkanConfig::device);		
		
		VkImageMemoryBarrier barrier{
			.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
			.srcAccessMask = 0,
			.dstAccessMask = 0,
			.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
			.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
			.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
			.image = textures.wood.image};

		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT; 
		barrier.subresourceRange.baseMipLevel = 0;
		barrier.subresourceRange.levelCount = 1;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1;

		VkPipelineStageFlags srcStage;
		VkPipelineStageFlags dstStage;
		
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

		srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;		
		vkCmdPipelineBarrier(
			commandBuffer,
			srcStage, dstStage,
			0,
			0, nullptr,
			0, nullptr,
			1, &barrier);	
		
		CommandBuffer::endSingleTimeCommands(commandBuffer, VulkanConfig::graphicsAndComputeQueue, VulkanConfig::device);		

		commandBuffer = CommandBuffer::beginSingleTimeCommands(VulkanConfig::device);	
		
		VkBufferImageCopy region{
			.bufferOffset = 0,
			.bufferRowLength = 0,
			.bufferImageHeight = 0,
			.imageOffset = {0,0,0},
			.imageExtent = {static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight), 1}
		};	

		region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		region.imageSubresource.mipLevel = 0;
		region.imageSubresource.baseArrayLayer = 0;
		region.imageSubresource.layerCount = 1;

		vkCmdCopyBufferToImage(
			commandBuffer,
			stagingBuffer,
			textures.wood.image,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			1,
			&region);	
	
		CommandBuffer::endSingleTimeCommands(commandBuffer, VulkanConfig::graphicsAndComputeQueue, VulkanConfig::device);

		commandBuffer = CommandBuffer::beginSingleTimeCommands(VulkanConfig::device);	
		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		
		srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;	
		dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;	
		vkCmdPipelineBarrier(
		commandBuffer,
		srcStage, dstStage,
		0,
		0, nullptr,
		0, nullptr,
		1, &barrier);	
		
		CommandBuffer::endSingleTimeCommands(commandBuffer, VulkanConfig::graphicsAndComputeQueue, VulkanConfig::device);		

		vkDestroyBuffer(VulkanConfig::device, stagingBuffer, nullptr);
		vkFreeMemory(VulkanConfig::device, stagingBufferMemory, nullptr);
	}	

	void setupImageViews()
	{
		VkImageViewCreateInfo viewInfo{
			.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
			.image = textures.wood.image,
			.viewType = VK_IMAGE_VIEW_TYPE_2D,
			.format = VK_FORMAT_R8G8B8A8_SRGB};		
			viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			viewInfo.subresourceRange.baseMipLevel = 0;
			viewInfo.subresourceRange.levelCount = 1;
			viewInfo.subresourceRange.baseArrayLayer = 0;
			viewInfo.subresourceRange.layerCount = 1;

		
		if (vkCreateImageView(VulkanConfig::device, &viewInfo, nullptr, &textures.wood.imageView))
		{
			throw std::runtime_error("failed to create image view!");	
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

	void setupLightDescriptorSets()
	{
		VkDescriptorPool descriptorPool;
		VkDescriptorSetLayoutBinding vertexLayoutBinding{
			.binding = 0,
			.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			.descriptorCount = 1,
			.stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
			.pImmutableSamplers = nullptr
		};

		std::array<VkDescriptorSetLayoutBinding, 1> setLayoutBindings{vertexLayoutBinding};
		
		VkDescriptorSetLayoutCreateInfo layoutInfo{
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
			.bindingCount = static_cast<uint32_t>(setLayoutBindings.size()),
			.pBindings = setLayoutBindings.data()
		};
	
		if (vkCreateDescriptorSetLayout(VulkanConfig::device, &layoutInfo, nullptr, &descriptorSetLayouts.light))
		{
			throw std::runtime_error("Failed to create descriptor set layout!");
		};	
		
		std::array<VkDescriptorPoolSize, 1> poolSizes{};
		poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		poolSizes[0].descriptorCount = static_cast<uint32_t>(frames * LIGHT_COUNT) * 2;

		VkDescriptorPoolCreateInfo poolInfo
		{
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
			.maxSets = static_cast<uint32_t>(frames * LIGHT_COUNT) * 2,
			.poolSizeCount = static_cast<uint32_t>(poolSizes.size()),
			.pPoolSizes = poolSizes.data()
		};	
		
		if (vkCreateDescriptorPool(VulkanConfig::device, &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create descriptor pool!");		
		}
		
		descriptorSets[0].light.resize(LIGHT_COUNT);
		descriptorSets[1].light.resize(LIGHT_COUNT);

		for (size_t j = 0; j < LIGHT_COUNT; j++)
		{
			std::array<VkDescriptorSetLayout, 1> layouts{};
			layouts.fill(descriptorSetLayouts.light);	
			
			VkDescriptorSetAllocateInfo allocInfo
			{
				.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
				.descriptorPool = descriptorPool,
				.descriptorSetCount = layouts.size(),
				.pSetLayouts = layouts.data()
			};

			for (size_t i = 0; i < frames; i++)
			{
				if (vkAllocateDescriptorSets(VulkanConfig::device, &allocInfo, &descriptorSets[i].light[j]) != VK_SUCCESS)
				{
					throw std::runtime_error("failed to allocate descriptor sets!");
				};
				
				VkDescriptorBufferInfo bufferInfo{
					.buffer = uniformBuffers[i].light[j],
					.offset = 0,
					.range = sizeof(LightUniform)
				};
				
				std::array<VkWriteDescriptorSet, 1> descriptorWrites{};
				
				descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				descriptorWrites[0].dstSet = descriptorSets[i].light[j];
				descriptorWrites[0].dstBinding = 0;
				descriptorWrites[0].dstArrayElement = 0;
				descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				descriptorWrites[0].descriptorCount = 1;
				descriptorWrites[0].pBufferInfo = &bufferInfo;

				vkUpdateDescriptorSets(VulkanConfig::device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(),0, nullptr);	
			};
		};
	}	

	void setupQuadDescriptorSets()
	{
		VkDescriptorPool descriptorPool;

		VkDescriptorSetLayoutBinding fragmentLayoutBinding
		{
			.binding = 0,
			.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			.descriptorCount = 1,
			.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
			.pImmutableSamplers = nullptr
		};

		VkDescriptorSetLayoutBinding sceneLayoutBinding
		{
			.binding = 1,
			.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			.descriptorCount = 1,
			.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
			.pImmutableSamplers = nullptr
		};

		std::array<VkDescriptorSetLayoutBinding, 2> setLayoutBindings{fragmentLayoutBinding, sceneLayoutBinding};
		
		VkDescriptorSetLayoutCreateInfo layoutInfo{
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
			.bindingCount = static_cast<uint32_t>(setLayoutBindings.size()),
			.pBindings = setLayoutBindings.data()
		};
	
		if (vkCreateDescriptorSetLayout(VulkanConfig::device, &layoutInfo, nullptr, &descriptorSetLayouts.quad))
		{
			throw std::runtime_error("Failed to create descriptor set layout!");
		};	
		
		std::array<VkDescriptorPoolSize, 2> poolSizes{};
		poolSizes[0].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

		poolSizes[0].descriptorCount = static_cast<uint32_t>(frames) * 2;
		poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

		poolSizes[1].descriptorCount = static_cast<uint32_t>(frames) * 2;


		VkDescriptorPoolCreateInfo poolInfo
		{
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
			.maxSets = static_cast<uint32_t>(frames) * 2,
			.poolSizeCount = static_cast<uint32_t>(poolSizes.size()),
			.pPoolSizes = poolSizes.data()
		};	
		
		if (vkCreateDescriptorPool(VulkanConfig::device, &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create descriptor pool!");		
		}
		
		std::array<VkDescriptorSetLayout, 1> layouts{};
		layouts.fill(descriptorSetLayouts.quad);	
		
		VkDescriptorSetAllocateInfo allocInfo
		{
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
			.descriptorPool = descriptorPool,
			.descriptorSetCount = layouts.size(),
			.pSetLayouts = layouts.data()
		};
		
		for (size_t i = 0; i < frames; i++)
		{
			if (vkAllocateDescriptorSets(VulkanConfig::device, &allocInfo, &descriptorSets[i].quad) != VK_SUCCESS)
			{
				throw std::runtime_error("failed to allocate descriptor sets!");
			};

			const std::string quadDSName{"descriptorSets[" + std::to_string(i) + "].quad"};
			setupDebugObjectName(VK_OBJECT_TYPE_DESCRIPTOR_SET, descriptorSets[i].quad, quadDSName.c_str());

			VkDescriptorImageInfo sceneInfo
			{
				.sampler = sampler,
				.imageView = textures.position.imageView,
				.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
			};
					
			VkDescriptorImageInfo imageInfo
			{
				.sampler = sampler,
				.imageView = textures.normal.imageView,
				.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
			};
			
			std::array<VkWriteDescriptorSet, 2> descriptorWrites{};
			descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[0].dstSet = descriptorSets[i].quad;
			descriptorWrites[0].dstBinding = 0;
			descriptorWrites[0].dstArrayElement = 0;
			descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			descriptorWrites[0].descriptorCount = 1;
			descriptorWrites[0].pImageInfo = &imageInfo;
			descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[1].dstSet = descriptorSets[i].quad;
			descriptorWrites[1].dstBinding = 1;
			descriptorWrites[1].dstArrayElement = 0;
			descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			descriptorWrites[1].descriptorCount = 1;
			descriptorWrites[1].pImageInfo = &sceneInfo;

			vkUpdateDescriptorSets(VulkanConfig::device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(),0, nullptr);	
		};
	}	

	void setupCubeDescriptorSets()
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
	
		if (vkCreateDescriptorSetLayout(VulkanConfig::device, &layoutInfo, nullptr, &descriptorSetLayouts.cube))
		{
			throw std::runtime_error("Failed to create descriptor set layout!");
		};	
		
		std::array<VkDescriptorPoolSize, 2> poolSizes{};
		poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		poolSizes[0].descriptorCount = static_cast<uint32_t>(frames) * 2;

		poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

		poolSizes[1].descriptorCount = static_cast<uint32_t>(frames) * 2;

		VkDescriptorPoolCreateInfo poolInfo
		{
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
			.maxSets = static_cast<uint32_t>(frames) * 2,
			.poolSizeCount = static_cast<uint32_t>(poolSizes.size()),
			.pPoolSizes = poolSizes.data()
		};	
		
		if (vkCreateDescriptorPool(VulkanConfig::device, &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create descriptor pool!");		
		}
		
		std::array<VkDescriptorSetLayout, 1> layouts{};
		layouts.fill(descriptorSetLayouts.cube);	
		
		VkDescriptorSetAllocateInfo allocInfo
		{
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
			.descriptorPool = descriptorPool,
			.descriptorSetCount = layouts.size(),
			.pSetLayouts = layouts.data()
		};
		
		for (size_t i = 0; i < frames; i++)
		{
			if (vkAllocateDescriptorSets(VulkanConfig::device, &allocInfo, &descriptorSets[i].cube) != VK_SUCCESS)
			{
				throw std::runtime_error("failed to allocate descriptor sets!");
			};
			
			VkDescriptorBufferInfo bufferInfo{
				.buffer = uniformBuffers[i].cube,
				.offset = 0,
				.range = sizeof(ObjectUniform)
			};
			
			VkDescriptorImageInfo imageInfo
			{
				.sampler = sampler,
				.imageView = textures.wood.imageView,
				.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
			};
			
			std::array<VkWriteDescriptorSet, 2> descriptorWrites{};
			
			descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[0].dstSet = descriptorSets[i].cube;
			descriptorWrites[0].dstBinding = 0;
			descriptorWrites[0].dstArrayElement = 0;
			descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			descriptorWrites[0].descriptorCount = 1;
			descriptorWrites[0].pBufferInfo = &bufferInfo;
			
			descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[1].dstSet = descriptorSets[i].cube;
			descriptorWrites[1].dstBinding = 1;
			descriptorWrites[1].dstArrayElement = 0;
			descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			descriptorWrites[1].descriptorCount = 1;
			descriptorWrites[1].pImageInfo = &imageInfo;

			vkUpdateDescriptorSets(VulkanConfig::device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(),0, nullptr);	
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

	void setupLightPipelineLayout()
	{
		VkPipelineLayoutCreateInfo pipelineLayoutInfo{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
			.setLayoutCount = 1,
			.pSetLayouts = &descriptorSetLayouts.light,
			.pushConstantRangeCount = 0,
			.pPushConstantRanges = nullptr
		};
	
		if (vkCreatePipelineLayout(VulkanConfig::device, &pipelineLayoutInfo, nullptr, &pipelineLayouts.light) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create pipeline layout!");
		};
	};

	void setupQuadPipelineLayout()
	{
		VkPipelineLayoutCreateInfo pipelineLayoutInfo{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
			.setLayoutCount = 1,
			.pSetLayouts = &descriptorSetLayouts.quad,
			.pushConstantRangeCount = 0,
			.pPushConstantRanges = nullptr
		};
	
		if (vkCreatePipelineLayout(VulkanConfig::device, &pipelineLayoutInfo, nullptr, &pipelineLayouts.quad) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create pipeline layout!");
		};
	};

	void setupModelPipelineLayout()
	{
		VkPipelineLayoutCreateInfo pipelineLayoutInfo{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
			.setLayoutCount = 1,
			.pSetLayouts = &model->layout,
			.pushConstantRangeCount = 0,
			.pPushConstantRanges = nullptr
		};
	
		if (vkCreatePipelineLayout(VulkanConfig::device, &pipelineLayoutInfo, nullptr, &pipelineLayouts.model) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create pipeline layout!");
		};
	};

	void setupCubePipelineLayout()
	{
		VkPipelineLayoutCreateInfo pipelineLayoutInfo{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
			.setLayoutCount = 1,
			.pSetLayouts = &descriptorSetLayouts.cube,
			.pushConstantRangeCount = 0,
			.pPushConstantRanges = nullptr
		};
	
		if (vkCreatePipelineLayout(VulkanConfig::device, &pipelineLayoutInfo, nullptr, &pipelineLayouts.cube) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create pipeline layout!");
		};
	};

	void setupLightPipeline()
	{
		setupLightPipelineLayout();
		
		shaderStages.clear();
		addShader(SHADER_DIRECTORY + "/deferred/pointLight/vert.spv", VK_SHADER_STAGE_VERTEX_BIT);	
		addShader(SHADER_DIRECTORY + "/deferred/pointLight/frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);	
	
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
			.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
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

		std::array<VkPipelineColorBlendAttachmentState, 4> colorBlendAttachments{colorBlendAttachment, colorBlendAttachment, colorBlendAttachment, colorBlendAttachment};

		VkPipelineColorBlendStateCreateInfo colorBlending{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
			.logicOpEnable = VK_FALSE,
			.logicOp = VK_LOGIC_OP_COPY,
			.attachmentCount = 4,
			.pAttachments = colorBlendAttachments.data(),
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
			.layout = pipelineLayouts.light,
			.renderPass = renderPasses.offscreenPass,
			.subpass = 0,
			.basePipelineHandle = VK_NULL_HANDLE
		};

		if (vkCreateGraphicsPipelines(VulkanConfig::device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipelines.light) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create primitive graphics pipeline!");
		}
	};

	void setupQuadPipeline()
	{
		setupQuadPipelineLayout();

		shaderStages.clear();
		addShader(SHADER_DIRECTORY + "/deferred/postProcessing/vert.spv", VK_SHADER_STAGE_VERTEX_BIT);	
		addShader(SHADER_DIRECTORY + "/deferred/postProcessing/frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);	
	
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
			.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
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
			.layout = pipelineLayouts.quad,
			.renderPass = renderPasses.renderPass,
			.subpass = 0,
			.basePipelineHandle = VK_NULL_HANDLE
		};

		if (vkCreateGraphicsPipelines(VulkanConfig::device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipelines.quad) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create primitive graphics pipeline!");
		}
	};

	void setupModelPipeline()
	{
		setupModelPipelineLayout();

		shaderStages.clear();
		addShader(SHADER_DIRECTORY + "/deferred/object/vert.spv", VK_SHADER_STAGE_VERTEX_BIT);	
		addShader(SHADER_DIRECTORY + "/deferred/object/frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);	
	
		VkVertexInputBindingDescription bindingDescription
		{
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
			.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
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

		std::array<VkPipelineColorBlendAttachmentState, 4> colorBlendAttachments{colorBlendAttachment, colorBlendAttachment, colorBlendAttachment, colorBlendAttachment};

		VkPipelineColorBlendStateCreateInfo colorBlending{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
			.logicOpEnable = VK_FALSE,
			.logicOp = VK_LOGIC_OP_COPY,
			.attachmentCount = 4,
			.pAttachments = colorBlendAttachments.data(),
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
			.layout = pipelineLayouts.model,
			.renderPass = renderPasses.offscreenPass,
			.subpass = 0,
			.basePipelineHandle = VK_NULL_HANDLE
		};

		if (vkCreateGraphicsPipelines(VulkanConfig::device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipelines.model) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create primitive graphics pipeline!");
		}
	};

	void setupCubePipeline()
	{
		setupCubePipelineLayout();

		addShader(SHADER_DIRECTORY + "/deferred/object/vert.spv", VK_SHADER_STAGE_VERTEX_BIT);	
		addShader(SHADER_DIRECTORY + "/deferred/object/frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);	
	
		VkVertexInputBindingDescription bindingDescription
		{
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
			.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
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

		std::array<VkPipelineColorBlendAttachmentState, 4> colorBlendAttachments{colorBlendAttachment, colorBlendAttachment, colorBlendAttachment, colorBlendAttachment};

		VkPipelineColorBlendStateCreateInfo colorBlending{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
			.logicOpEnable = VK_FALSE,
			.logicOp = VK_LOGIC_OP_COPY,
			.attachmentCount = 4,
			.pAttachments = colorBlendAttachments.data(),
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
			.layout = pipelineLayouts.cube,
			.renderPass = renderPasses.offscreenPass,
			.subpass = 0,
			.basePipelineHandle = VK_NULL_HANDLE
		};

		if (vkCreateGraphicsPipelines(VulkanConfig::device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipelines.cube) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create primitive graphics pipeline!");
		}
	};

	void updateUniformBuffer(uint32_t currentImage)
	{
		ObjectUniform uniformData;
		uint32_t index = 0;	
		for (int x = 0; x < 1; x++)
		{
			for (int z = 0; z < 1; z++)
			{
				uniformData.model = glm::mat4(1.);		
				uniformData.model = glm::translate(uniformData.model, glm::vec3(x * 5, 0., z * 5));
				uniformData.view = camera.getViewMatrix();
				uniformData.proj = glm::perspective(glm::radians(45.f), VulkanConfig::swapChainExtent.width / (float)VulkanConfig::swapChainExtent.height, 0.1f, FAR_PLANE);
				uniformData.proj[1][1] *= -1.;
				
				model->bind<ObjectUniform>(uniformData, currentImage, index);
				index++;
			};
		};

			// Light	
		for (size_t i = 0; i < LIGHT_COUNT; i++)
		{
			LightUniform lightUniform;
			lightUniform.model = glm::translate(glm::mat4(1.), lightPositions[i]);
			lightUniform.model = glm::scale(lightUniform.model, glm::vec3(.5));
			lightUniform.view = camera.getViewMatrix();
			lightUniform.proj = glm::perspective(glm::radians(45.f), VulkanConfig::swapChainExtent.width / (float)VulkanConfig::swapChainExtent.height, 0.1f, FAR_PLANE);
			lightUniform.proj[1][1] *= -1.;
			lightUniform.lightColor = lightColors[i];

			memcpy(uniformBuffersMapped[currentImage].light[i], &lightUniform, sizeof(lightUniform));
		};

		ObjectUniform objectUniform;
		objectUniform.model = glm::translate(glm::mat4(1.), glm::vec3(0., 0., 25.));		
		objectUniform.model = glm::scale(objectUniform.model, glm::vec3(2.5, 2.5, 27.5));		
		objectUniform.view = camera.getViewMatrix();
		objectUniform.proj = glm::perspective(glm::radians(45.f), VulkanConfig::swapChainExtent.width / (float)VulkanConfig::swapChainExtent.height, 0.1f, FAR_PLANE);
		objectUniform.cameraPos = camera.cameraPos;

		for (size_t i = 0; i < LIGHT_COUNT; i++)
		{
			objectUniform.lightPos[i] = lightPositions[i]; 
			objectUniform.lightColor[i] = lightColors[i]; 
		};

		objectUniform.proj[1][1] *= -1.;
		
		memcpy(uniformBuffersMapped[currentImage].cube, &objectUniform, sizeof(objectUniform));
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
		renderPassInfo.renderPass = renderPasses.offscreenPass;
		renderPassInfo.framebuffer = offscreenFramebuffers[imageIndex];
		renderPassInfo.renderArea.offset = { 0,0 };
		renderPassInfo.renderArea.extent = VulkanConfig::swapChainExtent;

		std::array<VkClearValue, 5> clearValues{};
		clearValues[0].depthStencil = {1.f, 0};
		clearValues[1].color = {{1.0f, .0f, .0f, 1.f}};
		clearValues[2].color = {{1.0f, .0f, .0f, 1.f}};
		clearValues[3].color = {{1.0f, .0f, .0f, 1.f}};
		clearValues[4].color = {{1.0f, .0f, .0f, 1.f}};

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

		vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

		VkBuffer vertexCubeBuffers[] = { vertexCubeBuffer };

		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelines.model);	
		
		for (uint32_t j = 0; j < 2; j++)
		{
			for (size_t i = 0; i < model->meshes.size(); i++)
			{
				const Mesh mesh = model->meshes[i];

				vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayouts.model, 0, 1, &model->descriptors[currentFrame].sets[j][i], 0, nullptr);
				
				vkCmdBindVertexBuffers(commandBuffer, 0, 1, &model->buffer.buffers[i], offsets);
				vkCmdBindIndexBuffer(commandBuffer, model->buffer.index[i], 0, VK_INDEX_TYPE_UINT32);

				vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(mesh.indices.size()), 1, 0, 0, 0);
			};
		};

		vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexCubeBuffers, offsets);
		
		vkCmdBindIndexBuffer(commandBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT32);		

		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelines.light);	

		for (size_t i = 0; i < LIGHT_COUNT; i++)
		{
			vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayouts.light, 0, 1, &descriptorSets[currentFrame].light[i], 0, nullptr);

			vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(DeferredRendering::cubeIndices.size()), 1, 0, 0, 0);
		};

		vkCmdEndRenderPass(commandBuffer);

		renderPassInfo.renderPass = renderPasses.renderPass;
		renderPassInfo.framebuffer = swapChainFramebuffers[imageIndex];

		vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelines.quad);	

		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayouts.quad, 0, 1, &descriptorSets[currentFrame].quad, 0, nullptr);

		vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexCubeBuffers, offsets);
		
		vkCmdBindIndexBuffer(commandBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT32);		

		//vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(DeferredRendering::quadIndices.size()), 1, 0, 0, 0);

		vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(DeferredRendering::cubeIndices.size()), 1, 0, 0, 0);

		vkCmdEndRenderPass(commandBuffer);

		if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to record command buffer!");
		}
	};

	void setupCubeResources()
	{
		setupCubeDescriptorSets();	
		setupCubePipeline();
	};

	void setupLightResources()
	{
		setupLightDescriptorSets();	
		setupLightPipeline();
	};

	void setupQuadResources()
	{
		setupQuadDescriptorSets();	
		setupQuadPipeline();
	};

	void init(GLFWwindow* window)
	{
		IVulkanApp::init(window);	
		setupOffscreenPass();
		setupRenderPass();
		setupSamplers();
		loadModel(); //model
		loadTexture();
		setupPosition();
		setupNormal();
		setupAlbedo();
		setupSpecular();
		setupDepth();
		setupImageViews();
		setupUniformBuffers();
		setupCubeResources();
		setupLightResources();
		setupQuadResources();
		setupModelDescriptorSets(); //model
		setupModelPipeline();
		createSwapChainFramebuffers();
		createOffscreenFramebuffers();
		setupDebugObjectNames();
	};

	void setupDebugObjectNames()	
	{
		setupDebugObjectName(VK_OBJECT_TYPE_IMAGE, textures.wood.image, "textures.wood.image");
		setupDebugObjectName(VK_OBJECT_TYPE_IMAGE, textures.position.image, "textures.position.image");
		setupDebugObjectName(VK_OBJECT_TYPE_IMAGE, textures.normal.image, "textures.normal.image");
		setupDebugObjectName(VK_OBJECT_TYPE_IMAGE, textures.albedo.image, "textures.albedo.image");
		setupDebugObjectName(VK_OBJECT_TYPE_IMAGE, textures.specular.image, "textures.specular.image");
		setupDebugObjectName(VK_OBJECT_TYPE_IMAGE, textures.depth.image, "textures.depth.image");

		setupDebugObjectName(VK_OBJECT_TYPE_IMAGE_VIEW, textures.wood.imageView, "textures.wood.imageView");
		setupDebugObjectName(VK_OBJECT_TYPE_IMAGE_VIEW, textures.position.imageView, "textures.position.imageView");
		setupDebugObjectName(VK_OBJECT_TYPE_IMAGE_VIEW, textures.normal.imageView, "textures.normal.imageView");
		setupDebugObjectName(VK_OBJECT_TYPE_IMAGE_VIEW, textures.albedo.imageView, "textures.albedo.imageView");
		setupDebugObjectName(VK_OBJECT_TYPE_IMAGE_VIEW, textures.specular.imageView, "textures.specular.imageView");
		setupDebugObjectName(VK_OBJECT_TYPE_IMAGE_VIEW, textures.depth.imageView, "textures.depth.imageView");

		for (size_t i = 0; i < swapChainImageViews.size(); i++)
		{
			const std::string offscreenFramebufferName{"offscreenFramebuffers[" + std::to_string(i) + "]"};
			const std::string swapChainFramebufferName{"swapChainFramebufferName[" + std::to_string(i) + "]"};

			setupDebugObjectName(VK_OBJECT_TYPE_FRAMEBUFFER, offscreenFramebuffers[i], offscreenFramebufferName.c_str());
			setupDebugObjectName(VK_OBJECT_TYPE_FRAMEBUFFER, swapChainFramebuffers[i], swapChainFramebufferName.c_str());
		};
	};

	void cleanup(GLFWwindow * window)
	{
		cleanupSwapChain();

		if (enableValidationLayers)
		{
			DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
		}

		for (size_t i = 0; i < frames; i++)
		{
			vkDestroyBuffer(VulkanConfig::device, uniformBuffers[i].cube, nullptr);
			vkFreeMemory(VulkanConfig::device, uniformBuffersMemory[i].cube, nullptr);
		}		

		vkDestroyDescriptorSetLayout(VulkanConfig::device, descriptorSetLayouts.cube, nullptr);

		vkDestroyBuffer(VulkanConfig::device, indexBuffer, nullptr);
		vkFreeMemory(VulkanConfig::device, indexBufferMemory, nullptr);
		
		vkDestroyBuffer(VulkanConfig::device, vertexCubeBuffer, nullptr);
		vkFreeMemory(VulkanConfig::device, vertexCubeBufferMemory, nullptr);
		vkDestroyBuffer(VulkanConfig::device, vertexCubeBuffer, nullptr);

		vkFreeMemory(VulkanConfig::device, vertexCubeBufferMemory, nullptr);

		vkDestroyImage(VulkanConfig::device, textures.wood.image, nullptr);
		vkFreeMemory(VulkanConfig::device, textures.wood.imageMemory, nullptr);

		vkDestroyRenderPass(VulkanConfig::device, renderPasses.offscreenPass, nullptr);

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
		setupPosition();
		setupNormal();
		setupAlbedo();
		setupSpecular();
		setupDepth();
		
		setupQuadDescriptorSets();

		createOffscreenFramebuffers();
		createSwapChainFramebuffers();
	}

};

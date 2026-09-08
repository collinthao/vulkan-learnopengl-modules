#include "vulkanApp.h"
#include "../../../debugUtils/debugUtils.h"

glm::vec3 IVulkanApp::cameraPos = glm::vec3(0., 0., 3.);
glm::vec3 IVulkanApp::cameraFront = glm::vec3(0.f, 0.f, -1.f);
glm::vec3 IVulkanApp::cameraUp = glm::vec3(0.f,1.f, 0.f);
Camera IVulkanApp::camera = Camera(IVulkanApp::cameraPos, IVulkanApp::cameraFront, IVulkanApp::cameraUp);

void IVulkanApp::init(GLFWwindow* window)
{
	createInstance();
	setupDebugMessenger();
	createSurface(window);
	pickPhysicalDevice();
	createLogicalDevice();
	createSwapChain(window);
	createImageViews();
	createRenderPass();
	CommandBuffer::createCommandPool(VulkanConfig::physicalDevice, VulkanConfig::device, surface);
	createVertexBuffers();
	createIndexBuffer();
	CommandBuffer::createCommandBuffers(VulkanConfig::device);
	createSyncObjects();
}

VkShaderModule IVulkanApp::createShaderModule(const std::vector<char>& code, VkDevice& device)
{
	VkShaderModuleCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = code.size();
	createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());
	
	VkShaderModule shaderModule;
	if (vkCreateShaderModule(VulkanConfig::device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
	{
		std::cout << "failed to create shader module!\n";
		throw std::runtime_error("failed to create shader module");
	}

	return shaderModule;
}

void IVulkanApp::createVertexBuffers()
{
	createVertexBuffer(cubeVertices, vertexCubeBuffer, vertexCubeBufferMemory);
}

void IVulkanApp::createIndexBuffer()
{
	VkDeviceSize bufferSize = sizeof(IVulkanApp::cubeIndices[0]) * IVulkanApp::cubeIndices.size();

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	Buffer::create(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory, VulkanConfig::device, VulkanConfig::physicalDevice);

	void* data;
	vkMapMemory(VulkanConfig::device, stagingBufferMemory, 0, bufferSize, 0, &data);
	memcpy(data, IVulkanApp::cubeIndices.data(), (size_t)bufferSize);
	vkUnmapMemory(VulkanConfig::device, stagingBufferMemory);

	Buffer::create(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, indexBuffer, indexBufferMemory, VulkanConfig::device, VulkanConfig::physicalDevice);
	copyBuffer(stagingBuffer, indexBuffer, bufferSize);
	
	vkDestroyBuffer(VulkanConfig::device, stagingBuffer, nullptr);
	vkFreeMemory(VulkanConfig::device, stagingBufferMemory, nullptr);
}

void IVulkanApp::createUniformBuffers()
{
	createGraphicsUniformBuffers();
}

void IVulkanApp::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
{
	VkCommandBuffer commandBuffer = CommandBuffer::beginSingleTimeCommands(VulkanConfig::device);

	VkBufferCopy copyRegion{};
	copyRegion.srcOffset = 0;
	copyRegion.dstOffset = 0;
	copyRegion.size = size;
	vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

	CommandBuffer::endSingleTimeCommands(commandBuffer, VulkanConfig::graphicsAndComputeQueue, VulkanConfig::device);
}

void IVulkanApp::createGraphicsUniformBuffers()
{
	VkDeviceSize bufferSize = sizeof(UniformBufferObjectModel);

	baseUniformBuffers.resize(VulkanConfig::MAX_FRAMES_IN_FLIGHT);
	baseUniformBuffersMemory.resize(VulkanConfig::MAX_FRAMES_IN_FLIGHT);
	baseUniformBuffersMapped.resize(VulkanConfig::MAX_FRAMES_IN_FLIGHT);

	for (size_t i = 0; i < VulkanConfig::MAX_FRAMES_IN_FLIGHT; i++)
	{
		Buffer::create(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
			baseUniformBuffers[i], baseUniformBuffersMemory[i], VulkanConfig::device, VulkanConfig::physicalDevice);

		vkMapMemory(VulkanConfig::device, baseUniformBuffersMemory[i], 0, bufferSize, 0, &baseUniformBuffersMapped[i]);
	}
}

void IVulkanApp::createSyncObjects()
{
	imageAvailableSemaphores.resize(VulkanConfig::MAX_FRAMES_IN_FLIGHT);
	renderFinishedSemaphores.resize(VulkanConfig::MAX_FRAMES_IN_FLIGHT);
	inFlightFences.resize(VulkanConfig::MAX_FRAMES_IN_FLIGHT);

	VkSemaphoreCreateInfo semaphoreInfo{};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkFenceCreateInfo fenceInfo{};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	for (size_t i = 0; i < VulkanConfig::MAX_FRAMES_IN_FLIGHT; i++)
	{
		if (vkCreateSemaphore(VulkanConfig::device, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
			vkCreateSemaphore(VulkanConfig::device, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS ||
			vkCreateFence(VulkanConfig::device, &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create semaphores!");
		}
	}
}

void IVulkanApp::createGraphicsDescriptorSets()
{
	std::vector<VkDescriptorSetLayout> layouts(VulkanConfig::MAX_FRAMES_IN_FLIGHT, Pipelines::basePipeline.descriptor.layout);
	VkDescriptorSetAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = Pipelines::basePipeline.descriptor.pool;
	allocInfo.descriptorSetCount = static_cast<uint32_t>(VulkanConfig::MAX_FRAMES_IN_FLIGHT);
	allocInfo.pSetLayouts = layouts.data();

	baseDescriptorSets.resize(VulkanConfig::MAX_FRAMES_IN_FLIGHT);
	if (vkAllocateDescriptorSets(VulkanConfig::device, &allocInfo, baseDescriptorSets.data()) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create descriptor sets!");
	}

	for (size_t i = 0; i < VulkanConfig::MAX_FRAMES_IN_FLIGHT; i++)
	{
		VkDescriptorBufferInfo bufferInfo{};
		bufferInfo.buffer = baseUniformBuffers[i];
		bufferInfo.offset = 0;
		bufferInfo.range = sizeof(UniformBufferObjectModel);
		VkDescriptorImageInfo imageInfo{};
		imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imageInfo.imageView = textureImageView;
		imageInfo.sampler = textureSampler;

		std::array<VkWriteDescriptorSet, 2> descriptorWrites{};
		descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[0].dstSet = baseDescriptorSets[i];
		descriptorWrites[0].dstBinding = 0;
		descriptorWrites[0].dstArrayElement = 0;
		descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorWrites[0].descriptorCount = 1;
		descriptorWrites[0].pBufferInfo = &bufferInfo;

		descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[1].dstSet = baseDescriptorSets[i];
		descriptorWrites[1].dstBinding = 1;
		descriptorWrites[1].dstArrayElement = 0;
		descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptorWrites[1].descriptorCount = 1;
		descriptorWrites[1].pImageInfo = &imageInfo;

		vkUpdateDescriptorSets(VulkanConfig::device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
	}
}

void IVulkanApp::cleanupSwapChain()
{
	for (auto framebuffer : swapChainFramebuffers)
	{
		vkDestroyFramebuffer(VulkanConfig::device, framebuffer, nullptr);
	}

	for (auto imageView : swapChainImageViews)
	{
		vkDestroyImageView(VulkanConfig::device, imageView, nullptr);
	}

	vkDestroySwapchainKHR(VulkanConfig::device, swapChain, nullptr);
}

void IVulkanApp::createFramebuffers()
{
	swapChainFramebuffers.resize(swapChainImageViews.size());

	for (size_t i = 0; i < swapChainImageViews.size(); i++)
	{
		std::array<VkImageView, 1> attachments = { 
			swapChainImageViews[i], 
		};
		
		VkFramebufferCreateInfo framebufferInfo{};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = basicRenderPasses.renderPass;
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

void IVulkanApp::createImageViews()
{
	swapChainImageViews.resize(swapChainImages.size());

	for (size_t i = 0; i < swapChainImages.size(); i++)
	{
		const std::string swapChainImageName{"swapChainImages[" + std::to_string(i) + "]"};
		const std::string swapChainImageViewName{"swapChainImageViews[" + std::to_string(i) + "]"};

		swapChainImageViews[i] = Image::createView(swapChainImages[i], swapChainImageViews[i], VK_IMAGE_VIEW_TYPE_2D, swapChainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1, 1, VulkanConfig::device, VulkanConfig::graphicsAndComputeQueue);
		setupDebugObjectName(VK_OBJECT_TYPE_IMAGE, swapChainImages[i], swapChainImageName);

		setupDebugObjectName(VK_OBJECT_TYPE_IMAGE_VIEW, swapChainImageViews[i], swapChainImageViewName);
	}
}

void IVulkanApp::recreateSwapChain(GLFWwindow * window)
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
	createFramebuffers();
}

VkSurfaceFormatKHR IVulkanApp::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
{
	for (const auto& availableFormat : availableFormats)
	{
		if (availableFormat.format == VK_FORMAT_R8G8B8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
		{
			return availableFormat;
		}
	}

	return availableFormats[0];
}

void IVulkanApp::createPipelines()
{
	Pipelines::createPipelines(VulkanConfig::device, basicRenderPasses); 
}

void IVulkanApp::createDescriptorSets()
{
	createGraphicsDescriptorSets();
}

void IVulkanApp::createRenderPass()
{
	VkAttachmentDescription colorAttachment{};
	colorAttachment.format = swapChainImageFormat;
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT; 
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference colorAttachmentRef{};
	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass{};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentRef;

	std::array<VkAttachmentDescription, 1> attachments = {colorAttachment};
	VkRenderPassCreateInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
	renderPassInfo.pAttachments = attachments.data();
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;

	if (vkCreateRenderPass(VulkanConfig::device, &renderPassInfo, nullptr, &basicRenderPasses.renderPass) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create render pass!");
	}
}

VkExtent2D IVulkanApp::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, GLFWwindow * window)
{
	if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
	{
		return capabilities.currentExtent;
	}
	else
	{
		int width, height;
		glfwGetFramebufferSize(window, &width, &height);

		VkExtent2D actualExtent =
		{
			static_cast<uint32_t>(width),
			static_cast<uint32_t>(height)
		};

		actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
		actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

		return actualExtent;

	}
}

void IVulkanApp::createSwapChain(GLFWwindow * window)
{
	SwapChainSupportDetails swapChainSupport = querySwapChainSupport(VulkanConfig::physicalDevice);

	VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
	VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
	VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities, window);

	uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;

	if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount)
	{
		imageCount = swapChainSupport.capabilities.maxImageCount;
	}

	VkSwapchainCreateInfoKHR createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = surface;
	createInfo.minImageCount = imageCount;
	createInfo.imageFormat = surfaceFormat.format;
	createInfo.imageColorSpace = surfaceFormat.colorSpace;
	createInfo.imageExtent = extent;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;

	QueueFamilyIndices indices = QueueFamily::findQueueFamilies(VulkanConfig::physicalDevice, surface);
	uint32_t queueFamilyIndices[] = {indices.graphicsAndComputeFamily.value(), indices.presentFamily.value()};

	if (indices.graphicsAndComputeFamily != indices.presentFamily)
	{
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices = queueFamilyIndices;
	}
	else
	{
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		createInfo.queueFamilyIndexCount = 0;
		createInfo.pQueueFamilyIndices = nullptr;
	}

	createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	createInfo.presentMode = presentMode;
	createInfo.clipped = VK_TRUE;
	createInfo.oldSwapchain = VK_NULL_HANDLE;

	if (vkCreateSwapchainKHR(VulkanConfig::device, &createInfo, nullptr, &swapChain) != VK_SUCCESS)
	{
		throw std::runtime_error("faild to create swap chain!");
	}

	vkGetSwapchainImagesKHR(VulkanConfig::device, swapChain, &imageCount, nullptr);
	swapChainImages.resize(imageCount);
	vkGetSwapchainImagesKHR(VulkanConfig::device, swapChain, &imageCount, swapChainImages.data());

	swapChainImageFormat = surfaceFormat.format;
	VulkanConfig::swapChainExtent = extent;
}

VkPresentModeKHR IVulkanApp::chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
{
	for (const auto& availablePresentMode : availablePresentModes)
	{
		if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
		{
			return availablePresentMode;
		}
	}

	return VK_PRESENT_MODE_FIFO_KHR;
}

void IVulkanApp::populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
{
	createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	createInfo.pfnUserCallback = debugCallback;
}

void IVulkanApp::setupDebugObjectName(VkObjectType objectType, void * handle, std::string name)
{
	VkDebugUtilsObjectNameInfoEXT nameInfo = {VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT};

	nameInfo.objectType = objectType;
	nameInfo.objectHandle = reinterpret_cast<uint64_t>(handle);
	nameInfo.pObjectName = name.c_str();
	
	if (CreateDebugUtilsObjectNameEXT(instance, VulkanConfig::device, &nameInfo) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to set up debug object name!");
	};
}

void IVulkanApp::setupDebugMessenger()
{
	VkDebugUtilsMessengerCreateInfoEXT createInfo;
	populateDebugMessengerCreateInfo(createInfo);

	if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to set up debug messenger!");
	};
}

int IVulkanApp::rateDeviceSuitability(VkPhysicalDevice device)
{
	VkPhysicalDeviceProperties deviceProperties;
	vkGetPhysicalDeviceProperties(device, &deviceProperties);

	VkPhysicalDeviceFeatures deviceFeatures;
	vkGetPhysicalDeviceFeatures(device, &deviceFeatures);
	
	int score = 0;

	if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) score += 1000;

	score += deviceProperties.limits.maxImageDimension2D;
	
	deviceFeatures.geometryShader = VK_TRUE;

	if (!deviceFeatures.geometryShader) 
	{
		std::cout << "Geometry shader not supported!\n";
		return 0;
	};

	return score;
}

bool IVulkanApp::isDeviceSuitable(VkPhysicalDevice device)
{
	QueueFamilyIndices indices = QueueFamily::findQueueFamilies(device, surface);

	bool extensionsSupported = checkDeviceExtensionSupport(device);

	bool swapChainAdequate = false;
	if (extensionsSupported)
	{
		SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
		swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
	}
	
	VkPhysicalDeviceFeatures supportedFeatures;
	vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

	return indices.isComplete() && extensionsSupported && swapChainAdequate && supportedFeatures.samplerAnisotropy;
}

VkSampleCountFlagBits IVulkanApp::getMaxUsableSampleCount()
{
	VkPhysicalDeviceProperties physicalDeviceProperties;
	vkGetPhysicalDeviceProperties(VulkanConfig::physicalDevice, &physicalDeviceProperties);

	VkSampleCountFlags counts = physicalDeviceProperties.limits.framebufferColorSampleCounts & physicalDeviceProperties.limits.framebufferDepthSampleCounts;

	if (counts & VK_SAMPLE_COUNT_64_BIT) { return VK_SAMPLE_COUNT_64_BIT;}
	if (counts & VK_SAMPLE_COUNT_32_BIT) { return VK_SAMPLE_COUNT_32_BIT;}
	if (counts & VK_SAMPLE_COUNT_16_BIT) { return VK_SAMPLE_COUNT_16_BIT;}
	if (counts & VK_SAMPLE_COUNT_8_BIT) { return VK_SAMPLE_COUNT_8_BIT;}
	if (counts & VK_SAMPLE_COUNT_4_BIT) { return VK_SAMPLE_COUNT_4_BIT;}
	if (counts & VK_SAMPLE_COUNT_2_BIT) { return VK_SAMPLE_COUNT_2_BIT;}

	return VK_SAMPLE_COUNT_1_BIT;
}

bool IVulkanApp::checkDeviceExtensionSupport(VkPhysicalDevice device)
{

	uint32_t extensionCount;
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

	std::vector<VkExtensionProperties> availableExtensions(extensionCount);
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

	std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

	for (const auto& extension : availableExtensions)
	{
		requiredExtensions.erase(extension.extensionName);
	}

	return requiredExtensions.empty();
}

void IVulkanApp::createLogicalDevice()
{
	QueueFamilyIndices indices = QueueFamily::findQueueFamilies(VulkanConfig::physicalDevice, surface);

	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	std::set<uint32_t> uniqueQueueFamilies = { indices.graphicsAndComputeFamily.value() , indices.presentFamily.value() };
	
	float queuePriority = 1.f;
	for (uint32_t queueFamily : uniqueQueueFamilies)
	{
	VkDeviceQueueCreateInfo queueCreateInfo{};
	queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queueCreateInfo.queueFamilyIndex = queueFamily;
	queueCreateInfo.queueCount = 1;
	queueCreateInfo.pQueuePriorities = &queuePriority;
	queueCreateInfos.push_back(queueCreateInfo);
	}

	VkPhysicalDeviceExtendedDynamicStateFeaturesEXT extendedDynamicStateFeatures{};
	extendedDynamicStateFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_FEATURES_EXT;
	extendedDynamicStateFeatures.extendedDynamicState = VK_TRUE;
	extendedDynamicStateFeatures.pNext = nullptr;

	VkDeviceCreateInfo createInfo{};
	VkPhysicalDeviceFeatures deviceFeatures{};
	deviceFeatures.samplerAnisotropy = VK_TRUE;
	deviceFeatures.sampleRateShading = VK_TRUE;
	deviceFeatures.geometryShader = VK_TRUE;
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	createInfo.pQueueCreateInfos = queueCreateInfos.data();
	createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
	createInfo.pEnabledFeatures = &deviceFeatures;
	createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
	createInfo.ppEnabledExtensionNames = deviceExtensions.data();
	createInfo.pNext = &extendedDynamicStateFeatures;

	if (vkCreateDevice(VulkanConfig::physicalDevice, &createInfo, nullptr, &VulkanConfig::device) != VK_SUCCESS)
	{
	throw std::runtime_error("failed to create logical device!");
	}

	vkGetDeviceQueue(VulkanConfig::device, indices.graphicsAndComputeFamily.value(), 0, &VulkanConfig::graphicsAndComputeQueue);
	vkGetDeviceQueue(VulkanConfig::device, indices.presentFamily.value(), 0, &presentQueue);
}

void IVulkanApp::pickPhysicalDevice()
{
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

	if (deviceCount == 0)
	{
		throw std::runtime_error("failed to find GPUs with Vulkan support!");
	}

	std::vector<VkPhysicalDevice> devices(deviceCount);
	vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

	std::multimap<int, VkPhysicalDevice> candidates;

	for (const auto& device : devices)
	{
		if (isDeviceSuitable(device))
		{
			int score = rateDeviceSuitability(device);
			candidates.insert(std::make_pair(score, device));
		}
	}

	if (candidates.rbegin()->first > 0)
	{
		VulkanConfig::physicalDevice = candidates.rbegin()->second;
		
		VulkanConfig::msaaSamples = getMaxUsableSampleCount();
		//VulkanConfig::msaaSamples = VK_SAMPLE_COUNT_1_BIT;
	}
	else
	{
		throw std::runtime_error("failed to find a suitable GPU");
	}
}

void IVulkanApp::createSurface(GLFWwindow * window)
{
	VkResult result = glfwCreateWindowSurface(instance, window, nullptr, &surface);

	std::cout << "Glfw result" << result << '\n';

	if (result != VK_SUCCESS)
	{
		std::cout << "Failed to create window surface\n";
		throw std::runtime_error("failed to create window surface!");
	}
}

std::vector<const char*> IVulkanApp::getRequiredExtensions()
{
	uint32_t glfwExtensionCount = 0;
	const char** glfwExtensions;
	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

	if (enableValidationLayers)
	{
		extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	}

	extensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);

	return extensions;
}

SwapChainSupportDetails IVulkanApp::querySwapChainSupport(VkPhysicalDevice device)
{
	SwapChainSupportDetails details;

	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

	uint32_t formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount,nullptr);

	if (formatCount != 0)
	{
	details.formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
	}

	uint32_t presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);

	if (presentModeCount != 0)
	{
		details.presentModes.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
	}

	return details;
}

void IVulkanApp::createInstance()
{
	VkApplicationInfo appInfo{};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "Vulkan Example";
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName = "No Engine";
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.apiVersion = VK_API_VERSION_1_0;

	VkInstanceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;

	auto extensions = getRequiredExtensions();

	createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
	createInfo.ppEnabledExtensionNames = extensions.data();

	VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
	if (enableValidationLayers)
	{
	createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
	createInfo.ppEnabledLayerNames = validationLayers.data();

	populateDebugMessengerCreateInfo(debugCreateInfo);
	createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
	}
	else
	{
		createInfo.enabledLayerCount = 0;
		createInfo.pNext = nullptr;
	}

	for (const auto& extension : extensions)
	{
		std::cout << "Extension: " << extension << '\n';
	};

	if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) throw std::runtime_error("failed to create instance!");
};

VkDevice* IVulkanApp::getDevice()
{
	return &VulkanConfig::device;
}

void IVulkanApp::updateUniformBuffer(uint32_t currentImage)
{
	for (size_t j = 0; j < 1; j++)
	{
		static auto startTime = std::chrono::high_resolution_clock::now();

		auto currentTime = std::chrono::high_resolution_clock::now();
		float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

		UniformBufferObject ubo{};
		UniformBufferObjectModel ubom{};

		glm::mat4 lightSpaceProjection = glm::ortho(-20.f, 20.f, -20.f, 20.f, 0.1f, 15.f);
		glm::mat4 lightView = glm::lookAt(
			glm::vec3(2.0f, 15.0f,1.0f), 
			glm::vec3(0.f, 0.f, 0.f),
			glm::vec3(0.f, 1.0f, 0.f)
		);

		glm::vec3 cubePosition = glm::vec3(1.);
		glm::vec3 transformedPosition = glm::vec3(cubePosition.x, cubePosition.y, cubePosition.z);
		ubom.model = glm::mat4(1.);

		ubom.model = glm::translate(ubom.model, transformedPosition);
		float angle = 20.f * j;

		ubom.model = glm::rotate(ubom.model, glm::radians(angle), glm::vec3(1.f, 0.3f, 0.5f));

		ubom.model = glm::scale(ubom.model, glm::vec3(1.));

		ubom.view = camera.getViewMatrix();
		ubom.proj = glm::perspective(glm::radians(45.f), VulkanConfig::swapChainExtent.width / (float)VulkanConfig::swapChainExtent.height, 0.1f, FAR_PLANE);

		lightSpaceProjection[1][1] *= -1;
		ubom.lightSpaceMatrix = lightSpaceProjection * lightView;
		ubom.fragColor = glm::vec3(0., 1., 1.);
	
		ubom.proj[1][1] *= -1;
		ubom.deltaTime = glfwGetTime();

		memcpy(baseUniformBuffersMapped[currentImage], &ubom, sizeof(ubom));
	}
}

void IVulkanApp::drawFrame(GLFWwindow * window)
{
	camera.update();
	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	vkWaitForFences(VulkanConfig::device, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

	uint32_t imageIndex;
	VkResult result = vkAcquireNextImageKHR(VulkanConfig::device, swapChain, UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);

	if (result == VK_ERROR_OUT_OF_DATE_KHR)
	{
		recreateSwapChain(window);
		return;
	}
	else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
	{
		throw std::runtime_error("failed to acquire swap chain image!");
	}

	vkResetFences(VulkanConfig::device, 1, &inFlightFences[currentFrame]);

	vkResetCommandBuffer(CommandBuffer::commandBuffers[currentFrame], 0);

	recordCommandBuffer(CommandBuffer::commandBuffers[currentFrame], imageIndex);
	
	updateUniformBuffer(currentFrame);

	VkSemaphore waitSemaphores[] = { imageAvailableSemaphores[currentFrame] };
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_VERTEX_INPUT_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &CommandBuffer::commandBuffers[currentFrame];

	VkSemaphore signalSemaphores[] = { renderFinishedSemaphores[currentFrame]};
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;

	if (vkQueueSubmit(VulkanConfig::graphicsAndComputeQueue, 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to submit draw command buffer!");
	}

	VkPresentInfoKHR presentInfo{};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signalSemaphores;

	VkSwapchainKHR swapChains[] = { swapChain };
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapChains;
	presentInfo.pImageIndices = &imageIndex;
	presentInfo.pResults = nullptr;

	result = vkQueuePresentKHR(presentQueue, &presentInfo);

	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || GLFWWindowContext::framebufferResized)
	{
		GLFWWindowContext::framebufferResized = false;
		recreateSwapChain(window);
	}
	else if (result != VK_SUCCESS)
	{
		throw std::runtime_error("failed to present swap chain image!");
	}

	currentFrame = (currentFrame + 1) % VulkanConfig::MAX_FRAMES_IN_FLIGHT;

	double currentTime = glfwGetTime();
	lastFrameTime = (currentTime - lastTime);
	lastTime = currentTime;
}

void IVulkanApp::recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex)
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
	renderPassInfo.renderPass = basicRenderPasses.renderPass;
	renderPassInfo.framebuffer = swapChainFramebuffers[imageIndex];
	renderPassInfo.renderArea.offset = { 0,0 };
	renderPassInfo.renderArea.extent = VulkanConfig::swapChainExtent;

	std::array<VkClearValue, 2> clearValues{};
	clearValues[0].color = {{1.f, 1.f, 1.f, 1.f}};
	clearValues[1].depthStencil = {.0f, 0};

	renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
	renderPassInfo.pClearValues = clearValues.data();

	vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

	VkDeviceSize offsets[] = { 0 };

	VkBuffer vertexCubeBuffers[] = { vertexCubeBuffer };
	
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

	Pipelines::basePipeline.bind(commandBuffer);

	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, Pipelines::basePipeline.getLayout(), 0, 1, &baseDescriptorSets[currentFrame], 0, nullptr);

	vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexCubeBuffers, offsets);

	vkCmdBindIndexBuffer(commandBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT32);

	vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(IVulkanApp::cubeIndices.size()), 1, 0, 0, 0);

	vkCmdEndRenderPass(commandBuffer);

	if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to record command buffer!");
	}
}

void IVulkanApp::moveCamera(double xpos, double ypos)
{
	camera.move(xpos, ypos);
}

void IVulkanApp::deviceWaitIdle()
{
	vkDeviceWaitIdle(VulkanConfig::device);
}

void IVulkanApp::createPostProcessingRenderPass()
{
	VkAttachmentDescription colorAttachment{};
	colorAttachment.format = swapChainImageFormat;
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT; 
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference colorAttachmentRef{};
	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass{};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentRef;
	subpass.pDepthStencilAttachment = nullptr;
	subpass.pResolveAttachments = nullptr;

	VkSubpassDependency dependency{};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
	dependency.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		
	std::array<VkAttachmentDescription, 1> attachments = {colorAttachment};
	VkRenderPassCreateInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
	renderPassInfo.pAttachments = attachments.data();
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	renderPassInfo.dependencyCount = 1;
	renderPassInfo.pDependencies = &dependency;

	if (vkCreateRenderPass(VulkanConfig::device, &renderPassInfo, nullptr, &basicRenderPasses.postProcessingRenderPass) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create render pass!");
	}
}
void IVulkanApp::createShadowMapRenderPass()
{
	VkAttachmentDescription depthAttachment{};
	//depthAttachment.format = findDepthFormat();
	depthAttachment.format = VK_FORMAT_D32_SFLOAT;
	depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT; 
	depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;

	VkAttachmentReference depthAttachmentRef{};
	depthAttachmentRef.attachment = 0;
	depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass{};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 0;
	subpass.pDepthStencilAttachment = &depthAttachmentRef;

	std::array<VkSubpassDependency, 2> dependency{};

	dependency[0].srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency[0].dstSubpass = 0;
	dependency[0].srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	dependency[0].srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
	dependency[0].dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	dependency[0].dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

	dependency[1].srcSubpass = 0;
	dependency[1].dstSubpass = VK_SUBPASS_EXTERNAL;
	dependency[1].srcStageMask = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
	dependency[1].srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
	dependency[1].dstStageMask =  VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	dependency[1].dstAccessMask =  VK_ACCESS_SHADER_READ_BIT;
	
	std::array<VkAttachmentDescription, 1> attachments = {depthAttachment};
	VkRenderPassCreateInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
	renderPassInfo.pAttachments = attachments.data();
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	renderPassInfo.dependencyCount = 2;
	renderPassInfo.pDependencies = dependency.data();

	if (vkCreateRenderPass(VulkanConfig::device, &renderPassInfo, nullptr, &basicRenderPasses.shadowMapRenderPass) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create render pass!");
	}
}

#ifndef VULKANRENDERER_H
#include "./renderer.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vector>
#include <stdexcept>
#include <map>
#include <set>
#include <unordered_map>
#include <optional>
#include <iostream>
#include <limits>
#include <algorithm>
#include <array>
#include <filesystem>
#include <fstream>
#include "../model.h"

#ifdef NDEBUG
	const bool enableValidationLayers = false;
#else
	const bool enableValidationLayers = true;
#endif
const std::vector<const char*> deviceExtensions =
{
	VK_KHR_SWAPCHAIN_EXTENSION_NAME,
	VK_EXT_EXTENDED_DYNAMIC_STATE_EXTENSION_NAME
};

struct SwapChainSupportDetails
{
	VkSurfaceCapabilitiesKHR capabilities;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> presentModes;
};

struct QueueFamilyIndices
{
	std::optional<uint32_t> graphicsAndComputeFamily;
	std::optional<uint32_t> presentFamily;

	bool isComplete()
	{
		return graphicsAndComputeFamily.has_value() && presentFamily.has_value();
	}
};

const std::vector<const char*> validationLayers =
{
	"VK_LAYER_KHRONOS_validation"
};

class VulkanRenderer : public IRenderer
{
	private:
	VkInstance instance;
	VkDebugUtilsMessengerEXT debugMessenger;
	VkSurfaceKHR surface;
	VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
	const int MAX_FRAMES_IN_FLIGHT = 2;
	VkDevice device;
	VkQueue graphicsAndComputeQueue;
	VkQueue presentQueue;
	VkSampleCountFlagBits msaaSamples = VK_SAMPLE_COUNT_8_BIT;
	VkSwapchainKHR swapChain;
	std::vector<VkImage> swapChainImages;
	std::vector<VkImage> offScreenImages;
	VkFormat swapChainImageFormat;
	VkExtent2D swapChainExtent;
	std::vector<VkImageView> swapChainImageViews;
	std::vector<VkImageView> offScreenImageViews;
	std::vector<VkDeviceMemory> offScreenImageMemories;
	VkRenderPass renderPass;
	VkRenderPass postProcessingRenderPass;
	VkImage textureImage;
	VkDeviceMemory textureImageMemory;
	VkDeviceMemory cubemapImageMemory;
	VkImageView textureImageView;
	VkSampler textureSampler;
	VkDescriptorSetLayout descriptorSetLayout;
	VkDescriptorSetLayout primitiveDescriptorSetLayout;
	VkDescriptorSetLayout stencilDescriptorSetLayout;
	VkDescriptorSetLayout modelDescriptorSetLayout;
	VkDescriptorSetLayout computeDescriptorSetLayout;
	VkDescriptorSetLayout lightDescriptorSetLayout;
	VkDescriptorSetLayout postProcessingDescriptorSetLayout;
	VkDescriptorSetLayout cubemapDescriptorSetLayout;
	
	VkPipelineLayout pipelineLayout;
	VkPipelineLayout computePipelineLayout;
	VkPipelineLayout modelPipelineLayout;
	VkPipelineLayout stencilPipelineLayout;
	VkPipelineLayout primitivePipelineLayout;
	VkPipelineLayout cubemapPipelineLayout;
	VkPipelineLayout lightPipelineLayout;
	VkPipelineLayout postProcessingPipelineLayout;
	
	VkPipeline graphicsPipeline;
	VkPipeline modelGraphicsPipeline;
	VkPipeline stencilPipeline;
	VkPipeline primitiveObjectPipeline;
	VkPipeline cubemapPipeline;
	VkPipeline lightPipeline;
	VkPipeline computePipeline;
	VkPipeline postProcessingPipeline;
	VkCommandPool commandPool;

	VkImage depthImage;
	VkDeviceMemory depthImageMemory;
	VkImageView depthImageView;

	VkImage colorImage;
	VkDeviceMemory colorImageMemory;
	VkImageView colorImageView;

	std::vector<VkFramebuffer> swapChainFramebuffers;
	std::vector<VkFramebuffer> offScreenFramebuffers;

	Model * model;

	std::vector<VkBuffer> vertexBuffers;
	std::vector<VkDeviceMemory> vertexBufferMemories;

	const std::string MODEL_PATH = "models/Sponza-master/sponza.obj";
	size_t MESH_COUNT = 0;

	void createInstance();
	void createSurface(GLFWwindow * window);
	void setupDebugMessenger();
	void pickPhysicalDevice();
	void createLogicalDevice();
	void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
	bool isDeviceSuitable(VkPhysicalDevice device);
	int rateDeviceSuitability(VkPhysicalDevice device);
	bool checkDeviceExtensionSupport(VkPhysicalDevice device);
	void createSwapChain(GLFWwindow * window);
	void createImageViews();
	void createRenderPass();
	void createPostProcessingRenderPass();
	void createDescriptorSetLayouts();	
	void createDescriptorSetLayout();
	void createPrimitiveDescriptorSetLayout();
	void createStencilDescriptorSetLayout();
	void createModelDescriptorSetLayout();
	void createPostProcessingDescriptorSetLayout();
	void createCubemapDescriptorSetLayout();
	void createLightDescriptorSetLayout();
	void createComputeDescriptorSetLayout();
	void createPipelines();
	void createGraphicsPipeline();
	void createPrimitivePipeline();
	void createStencilPipeline();
	void createLightPipeline();
	void createModelGraphicsPipeline();
	void createPostProcessingPipeline();
	void createCubemapPipeline();
	void createComputePipeline();
	void createCommandPool();
	void createOffscreenResources();
	void createImage(uint32_t width, uint32_t height, uint32_t mipLevels, uint32_t arrayLayers, VkImageCreateFlags flags, VkImageType imageType,VkSampleCountFlagBits numSamples,VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory, VkImageLayout imageLayout);
	uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
	void createColorResources();
	void createDepthResources();
	void createFramebuffers();
	void createModel();

	VkShaderModule createShaderModule(const std::vector<char>& code);

	VkImageView createImageView(VkImage image, VkImageView imageView, VkImageViewType viewType, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels, uint32_t layerCount);
	VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
	VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
	VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, GLFWwindow * window);
	SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);
	VkSampleCountFlagBits getMaxUsableSampleCount();
	QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
	std::vector<const char*>getRequiredExtensions();

	public:
	VulkanRenderer();

	void init(GLFWwindow * window);
	void drawFrame();

	static VkResult CreateDebugUtilsMessengerEXT(
		VkInstance instance, 
		const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, 
		const VkAllocationCallbacks* pAllocator, 
		VkDebugUtilsMessengerEXT* pDebugMessenger)
	{
		auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
		if (func != nullptr)
		{
			return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
		}
		else
		{
			return VK_ERROR_EXTENSION_NOT_PRESENT;
		}

	}

	void static getInstance();
	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData
	)
	{
		std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;
		return VK_FALSE;
	};

	static std::vector<char> readFile(const std::string& filename)
	{

		std::ifstream file(filename, std::ios::ate | std::ios::binary);

		if (!file.is_open())
		{
			throw std::runtime_error("failed to open file!");
		}

		size_t fileSize = (size_t)file.tellg();
		std::vector<char> buffer(fileSize);

		file.seekg(0);
		file.read(buffer.data(), fileSize);

		file.close();

		return buffer;

	}
};

#endif

#ifndef VULKANAPP_H
#define VULKANAPP_H

#include <assimp/mesh.h>
#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>

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
#include <random>
#include "../model.h"
#include <stb_image.h>
#include <tiny_obj_loader.h>
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <gtx/hash.hpp>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include "../camera.h"
#include "../pipeline/pipelineBuilder.h"

namespace fs = std::filesystem;

struct Material
{
	alignas(16)glm::vec3 specular;
	alignas(4)float shininess;
};

struct DirectionalLight
{
	alignas(16)glm::vec3 ambient;
	alignas(16)glm::vec3 diffuse;
	alignas(16)glm::vec3 specular;
	alignas(16)glm::vec3 direction;
};

struct SpotLight
{
	alignas(16)glm::vec3 ambient;
	alignas(16)glm::vec3 diffuse;
	alignas(16)glm::vec3 specular;
	alignas(16)glm::vec3 position;
	alignas(16)glm::vec3 direction;
	alignas(4)float cutOff;
	alignas(4)float outerCutOff;
};

struct PointLight
{
	alignas(16)glm::mat4 model;
	alignas(16)glm::mat4 view;
	alignas(16)glm::mat4 projection;
	alignas(16)glm::vec3 ambient;
	alignas(16)glm::vec3 diffuse;
	alignas(16)glm::vec3 specular;
	alignas(16)glm::vec3 position;
	alignas(16)glm::vec3 color;
	alignas(4)float constant;
	alignas(4)float linear;
	alignas(4)float quadratic;
};

struct Lights
{
	alignas(16)PointLight pointLights[4];	
	alignas(16)DirectionalLight directionalLight;
	alignas(16)SpotLight spotLight;
};

struct Light 
{
	alignas(16)glm::vec3 ambient;
	alignas(16)glm::vec3 diffuse;
	alignas(16)glm::vec3 specular;
	alignas(16)glm::vec3 position;
	alignas(16)glm::vec3 direction;
	alignas(4)float constant;
	alignas(4)float linear;
	alignas(4)float quadratic;
	alignas(4)float cutOff;
	alignas(4)float outerCutOff;
};

struct UniformBufferObjectModel
{
	 alignas(16) glm::mat4 model;
	 alignas(16) glm::mat4 view;
	 alignas(16) glm::mat4 proj;
	 alignas(16) glm::vec3 fragColor;
	 alignas(16) glm::vec3 cameraPos;
	 alignas(4)  int matIndex;
};

struct UniformBufferObject
{
	float deltaTime;
};

namespace std 
{
	template<> struct hash<Vertex>
	{
		size_t operator()(Vertex const& vertex) const
		{
			return ((hash<glm::vec3>()(vertex.pos) ^
				(hash<glm::vec3>()(vertex.color) << 1)) >> 1) ^
				(hash<glm::vec3>()(vertex.normal) << 1) >> 1 ^
				(hash<glm::vec2>()(vertex.texCoord) << 1);
		}
	};
};

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

class VulkanApp
{
	private:
	VkInstance instance;
	PipelineBuilder pipelineBuilder{};
	VkDebugUtilsMessengerEXT debugMessenger;
	VkSurfaceKHR surface;
	VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
	const int MAX_FRAMES_IN_FLIGHT = 2;
	VkQueue graphicsAndComputeQueue;
	VkQueue presentQueue;
	VkSampleCountFlagBits msaaSamples = VK_SAMPLE_COUNT_8_BIT;
	VkSwapchainKHR swapChain;
	std::vector<VkImage> swapChainImages;
	std::vector<VkImage> offScreenImages;
	VkFormat swapChainImageFormat;
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
	
	Pipeline primitivePipeline; 

	VkPipeline graphicsPipeline;
	VkPipeline modelGraphicsPipeline;
	VkPipeline stencilPipeline;
	VkPipeline primitiveObjectPipeline;
	VkPipeline cubemapPipeline;
	VkPipeline lightPipeline;
	VkPipeline computePipeline;
	VkPipeline postProcessingPipeline;
	VkCommandPool commandPool;

	VkImage cubemapImage;
	VkImageView cubemapImageView;

	VkImage depthImage;
	VkDeviceMemory depthImageMemory;
	VkImageView depthImageView;

	VkImage colorImage;
	VkDeviceMemory colorImageMemory;
	VkImageView colorImageView;

	VkImage specularImage;
	VkDeviceMemory specularImageMemory;
	VkImageView specularImageView;
	VkSampler specularSampler;
	
	VkBuffer indexBuffer;
	VkDeviceMemory indexBufferMemory;
		
	std::vector<VkBuffer> indexModelBuffers;
	std::vector<VkDeviceMemory> indexModelBufferMemories;
	
	std::vector<VkImage> modelImages;
	std::vector<VkDeviceMemory> modelImageMemories;
	std::vector<VkImageView> modelImageViews;
	std::vector<VkSampler> modelSamplers;
	std::vector<VkFramebuffer> swapChainFramebuffers;
	std::vector<VkFramebuffer> offScreenFramebuffers;

	VkBuffer vertexCubeBuffer;
	VkBuffer vertexCubemapBuffer;
	VkDeviceMemory vertexCubeBufferMemory;
	VkDeviceMemory vertexCubemapBufferMemory;

	std::vector<VkBuffer> uniformBuffers;
	std::vector<VkBuffer> cubemapUniformBuffers;
	std::vector<std::vector<VkBuffer>> modelUniformBuffers;
	std::vector<std::vector<VkBuffer>> primitiveUniformBuffers;
	std::vector<std::vector<VkBuffer>> stencilUniformBuffers;
	std::vector<std::vector<VkBuffer>> materialUniformBuffers;
	std::vector<std::vector<VkBuffer>> lightUniformBuffers;
	std::vector<std::vector<VkBuffer>> modelLightUniformBuffers;
	std::vector<std::vector<VkBuffer>> lightObjectUniformBuffers;

	std::vector<VkDeviceMemory> uniformBuffersMemory;
	std::vector<VkDeviceMemory> cubemapUniformBuffersMemory;
	std::vector<std::vector<VkDeviceMemory>> modelUniformBuffersMemory;
	std::vector<std::vector<VkDeviceMemory>> materialUniformBuffersMemory;
	std::vector<std::vector<VkDeviceMemory>> primitiveUniformBuffersMemory;
	std::vector<std::vector<VkDeviceMemory>> stencilUniformBuffersMemory;
	std::vector<std::vector<VkDeviceMemory>> lightUniformBuffersMemory;
	std::vector<std::vector<VkDeviceMemory>> modelLightUniformBuffersMemory;
	std::vector<std::vector<VkDeviceMemory>> lightObjectUniformBuffersMemory;

	std::vector<void*> uniformBuffersMapped;
	std::vector<void*> cubemapUniformBuffersMapped;
	std::vector<std::vector<void*>> modelUniformBuffersMapped;
	std::vector<std::vector<void*>> materialUniformBuffersMapped;
	std::vector<std::vector<void*>> primitiveUniformBuffersMapped;
	std::vector<std::vector<void*>> stencilUniformBuffersMapped;
	std::vector<std::vector<void*>> modelLightUniformBuffersMapped;
	std::vector<std::vector<void*>> lightUniformBuffersMapped;
	std::vector<std::vector<void*>> lightObjectUniformBuffersMapped;

	VkDescriptorPool descriptorPool;
	VkDescriptorPool computeDescriptorPool;
	VkDescriptorPool modelDescriptorPool;
	VkDescriptorPool primitiveDescriptorPool;
	VkDescriptorPool stencilDescriptorPool;
	VkDescriptorPool lightDescriptorPool;
	VkDescriptorPool postProcessingDescriptorPool;
	VkDescriptorPool cubemapDescriptorPool;

	std::vector<VkDescriptorSet> descriptorSets;
	std::vector<VkDescriptorSet> postProcessingDescriptorSets;
	std::vector<VkDescriptorSet> cubemapDescriptorSets;
	std::vector<VkDescriptorSet> computeDescriptorSets;
	std::vector<std::vector<VkDescriptorSet>> modelDescriptorSets;
	std::vector<std::vector<VkDescriptorSet>> primitiveDescriptorSets;
	std::vector<std::vector<VkDescriptorSet>> stencilDescriptorSets;
	std::vector<std::vector<VkDescriptorSet>> lightDescriptorSets;

	Model * model;

	std::vector<VkBuffer> vertexBuffers;
	std::vector<VkDeviceMemory> vertexBufferMemories;

	std::vector<VkBuffer> shaderStorageBuffers;
	std::vector<VkDeviceMemory> shaderStorageBuffersMemory;
	
	VkBuffer quadIndexBuffer;
	VkDeviceMemory quadIndexBufferMemory;

	std::vector<Vertex> modelVertices;
	std::vector<uint32_t> indices;

	std::vector<VkCommandBuffer> commandBuffers;
	std::vector<VkCommandBuffer> computeCommandBuffers;
	
	std::vector<VkSemaphore> imageAvailableSemaphores;
	std::vector<VkSemaphore> renderFinishedSemaphores;
	std::vector<VkFence> inFlightFences;
	
	std::vector<VkFence> computeInFlightFences;
	std::vector<VkSemaphore> computeFinishedSemaphores;



	PointLight pointLights[4];

	Lights lights;

	uint32_t currentFrame = 0;

	uint32_t mipLevels;

	const std::string MODEL_PATH = "models/Sponza-master/sponza.obj";
	const std::string MODEL_TEXTURE_DIRECTORY = "models/Sponza-master/";
	const std::string TEXTURE_PATH = "textures/container.png";
	const std::string CUBEMAP_PATH = "textures/skybox/";
	const std::string SPECULAR_PATH = "textures/container_specular.png";
	size_t MESH_COUNT = 0;
	const uint32_t PARTICLE_COUNT = 8192;
	const uint32_t OBJECT_COUNT = 10;
	const uint32_t MAX_POINT_LIGHTS = 4;
	const float FAR_PLANE = 400.f;
	float lastFrameTime = 0.f;
	double lastTime = 0.f;

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
	void createTextureImageView(VkImage& image, VkImageView& imageView);
	void createTextureImage(const std::string imagePath, VkImage& image, VkDeviceMemory& imageMemory);
	void loadTexture(const std::string imagePath, int& width, int& height, int& channels, int layers,stbi_uc*& texture);
	void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage , VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
	void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels, uint32_t layerCount);
	void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, uint32_t layerCount);
	void generateMipmaps(VkImage image, VkFormat imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels, uint32_t layerCount);
	void endSingleTimeCommands(VkCommandBuffer commandBuffer);
	bool hasStencilComponent(VkFormat format);
	VkCommandBuffer beginSingleTimeCommands();
	VkShaderModule createShaderModule(const std::vector<char>& code);
	VkImageView createImageView(VkImage image, VkImageView imageView, VkImageViewType viewType, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels, uint32_t layerCount);
	VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
	VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
	VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, GLFWwindow * window);
	SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);
	VkSampleCountFlagBits getMaxUsableSampleCount();
	QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
	std::vector<const char*>getRequiredExtensions();
	void createCubeTextureImage(const std::string imagePath, VkImage& image, VkDeviceMemory& imageMemory);
	void createCubeMapResources();
	void createTextureSampler(VkSampler& sampler);
	void createTextureImages(std::vector<VkImage>& images, std::vector<VkDeviceMemory>& imageMemories);
	void createTextureImageViews(std::vector<VkImage>& images, std::vector<VkImageView>& imageViews);
	void createTextureSamplers(std::vector<VkSampler>& samplers);
	void loadModel();
	void createShaderStorageBuffers();
	void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
	void createVertexBuffers();
	void createVertexBuffer(std::vector<Vertex> vertices, VkBuffer& buffer, VkDeviceMemory& memory);
	void createIndexBuffer();
	void createQuadIndexBuffer();
	void createModelIndexBuffers();
	void createModelIndexBuffer(std::vector<uint32_t> m_Indices, VkBuffer& modelBuffer,VkDeviceMemory& modelMemory);
	void createUniformBuffers();
	void createGraphicsUniformBuffers();
	void createPrimitiveUniformBuffers();
	void createCubemapUniformBuffers();
	void createStencilUniformBuffers();
	void createMaterialUniformBuffers();
	void createLightUniformBuffers();
	void createModelLightUniformBuffers();
	void createLightObjectUniformBuffers();
	void createModelUniformBuffers();
	void createDescriptorPools();
	void createGraphicsDescriptorPool();
	void createPrimitiveDescriptorPool();
	void createStencilDescriptorPool();
	void createModelDescriptorPool();
	void createPostProcessingDescriptorPool();
	void createCubemapDescriptorPool();
	void createLightDescriptorPool();
	void createComputeDescriptorPool();
	void createDescriptorSets();
	void createGraphicsDescriptorSets();
	void createPrimitiveDescriptorSets();
	void createStencilDescriptorSets();
	void createModelDescriptorSets();
	void createPostProcessingDescriptorSets();
	void createCubemapDescriptorSets();
	void createLightDescriptorSets();
	void createComputeDescriptorSets();
	void createCommandBuffers();
	void createComputeCommandBuffers();
	void createSyncObjects();
	void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);
	void updateUniformBuffer(uint32_t currentImage);
	void recordComputeCommandBuffer(VkCommandBuffer commandBuffer);
	void recreateSwapChain(GLFWwindow * window);
	void cleanupSwapChain();

	public:
	VulkanApp();

	void init(GLFWwindow * window);
	void drawFrame(GLFWwindow * window);
	void processInput(GLFWwindow * window);
	void cleanup(GLFWwindow * window);
	void deviceWaitIdle();

	static Camera camera;
	static glm::vec3 cameraPos;
	static glm::vec3 cameraFront;
	static glm::vec3 cameraUp; 
	static VkExtent2D swapChainExtent;

	VkDevice* getDevice();
	VkDevice device;

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


	static void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator)
	{
		auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");

		if (func != nullptr)
		{
			func(instance, debugMessenger, pAllocator);
		}
	}

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

	static inline glm::vec3 cubePositions[] = {
		glm::vec3( 0.0f,  0.0f,  0.0f),
		glm::vec3( 2.0f,  5.0f, -15.0f),
		glm::vec3(-1.5f, -2.2f, -2.5f),
		glm::vec3(-3.8f, -2.0f, -12.3f),
		glm::vec3( 2.4f, -0.4f, -3.5f),
		glm::vec3(-1.7f,  3.0f, -7.5f),
		glm::vec3( 1.3f, -2.0f, -2.5f),
		glm::vec3( 1.5f,  2.0f, -2.5f),
		glm::vec3( 1.5f,  0.2f, -1.5f),
		glm::vec3(-1.3f,  1.0f, -1.5f)
	};

	static inline const std::vector<Vertex> cubemapVertices = {
	    {{-1.0f,  1.0f, -1.0f},{0.5f,0.5f,0.5f},{0.0f,0.0f,-1.0f},{0.0f,0.0f}},
	    {{-1.0f, -1.0f, -1.0f},{0.5f,0.5f,0.5f},{0.0f,0.0f,-1.0f},{1.0f,0.0f}},
	    {{ 1.0f, -1.0f, -1.0f},{0.5f,0.5f,0.5f},{0.0f,0.0f,-1.0f},{1.0f,1.0f}},
	    {{ 1.0f, -1.0f, -1.0f},{0.5f,0.5f,0.5f},{0.0f,0.0f,-1.0f},{1.0f,1.0f}},
	    {{ 1.0f,  1.0f, -1.0f},{0.5f,0.5f,0.5f},{0.0f,0.0f,-1.0f},{0.0f,1.0f}},
	    {{-1.0f,  1.0f, -1.0f},{0.5f,0.5f,0.5f},{0.0f,0.0f,-1.0f},{0.0f,0.0f}},

	    {{-1.0f, -1.0f,  1.0f},{0.5f,0.5f,0.5f},{0.0f,0.0f, 1.0f},{0.0f,0.0f}},
	    {{-1.0f, -1.0f, -1.0f},{0.5f,0.5f,0.5f},{0.0f,0.0f, 1.0f},{1.0f,0.0f}},
	    {{-1.0f,  1.0f, -1.0f},{0.5f,0.5f,0.5f},{0.0f,0.0f, 1.0f},{1.0f,1.0f}},
	    {{-1.0f,  1.0f, -1.0f},{0.5f,0.5f,0.5f},{0.0f,0.0f, 1.0f},{1.0f,1.0f}},
	    {{-1.0f,  1.0f,  1.0f},{0.5f,0.5f,0.5f},{0.0f,0.0f, 1.0f},{0.0f,1.0f}},
	    {{-1.0f, -1.0f,  1.0f},{0.5f,0.5f,0.5f},{0.0f,0.0f, 1.0f},{0.0f,0.0f}},

	    {{ 1.0f, -1.0f, -1.0f},{0.5f,0.5f,0.5f},{1.0f,0.0f,0.0f},{1.0f,0.0f}},
	    {{ 1.0f, -1.0f,  1.0f},{0.5f,0.5f,0.5f},{1.0f,0.0f,0.0f},{1.0f,1.0f}},
	    {{ 1.0f,  1.0f,  1.0f},{0.5f,0.5f,0.5f},{1.0f,0.0f,0.0f},{0.0f,1.0f}},
	    {{ 1.0f,  1.0f,  1.0f},{0.5f,0.5f,0.5f},{1.0f,0.0f,0.0f},{0.0f,1.0f}},
	    {{ 1.0f,  1.0f, -1.0f},{0.5f,0.5f,0.5f},{1.0f,0.0f,0.0f},{0.0f,0.0f}},
	    {{ 1.0f, -1.0f, -1.0f},{0.5f,0.5f,0.5f},{1.0f,0.0f,0.0f},{1.0f,0.0f}},

	    {{-1.0f, -1.0f,  1.0f},{0.5f,0.5f,0.5f},{0.0f,-1.0f,0.0f},{0.0f,1.0f}},
	    {{-1.0f,  1.0f,  1.0f},{0.5f,0.5f,0.5f},{0.0f,-1.0f,0.0f},{1.0f,1.0f}},
	    {{ 1.0f,  1.0f,  1.0f},{0.5f,0.5f,0.5f},{0.0f,-1.0f,0.0f},{1.0f,0.0f}},
	    {{ 1.0f,  1.0f,  1.0f},{0.5f,0.5f,0.5f},{0.0f,-1.0f,0.0f},{1.0f,0.0f}},
	    {{ 1.0f, -1.0f,  1.0f},{0.5f,0.5f,0.5f},{0.0f,-1.0f,0.0f},{0.0f,0.0f}},
	    {{-1.0f, -1.0f,  1.0f},{0.5f,0.5f,0.5f},{0.0f,-1.0f,0.0f},{0.0f,1.0f}},

	    {{-1.0f,  1.0f, -1.0f},{0.5f,0.5f,0.5f},{0.0f,1.0f,0.0f},{0.0f,1.0f}},
	    {{ 1.0f,  1.0f, -1.0f},{0.5f,0.5f,0.5f},{0.0f,1.0f,0.0f},{1.0f,1.0f}},
	    {{ 1.0f,  1.0f,  1.0f},{0.5f,0.5f,0.5f},{0.0f,1.0f,0.0f},{1.0f,0.0f}},
	    {{ 1.0f,  1.0f,  1.0f},{0.5f,0.5f,0.5f},{0.0f,1.0f,0.0f},{1.0f,0.0f}},
	    {{-1.0f,  1.0f,  1.0f},{0.5f,0.5f,0.5f},{0.0f,1.0f,0.0f},{0.0f,0.0f}},
	    {{-1.0f,  1.0f, -1.0f},{0.5f,0.5f,0.5f},{0.0f,1.0f,0.0f},{0.0f,1.0f}},

	    {{-1.0f, -1.0f, -1.0f},{0.5f,0.5f,0.5f},{0.0f,-1.0f,0.0f},{0.0f,1.0f}},
	    {{-1.0f, -1.0f,  1.0f},{0.5f,0.5f,0.5f},{0.0f,-1.0f,0.0f},{1.0f,1.0f}},
	    {{ 1.0f, -1.0f, -1.0f},{0.5f,0.5f,0.5f},{0.0f,-1.0f,0.0f},{1.0f,0.0f}},
	    {{ 1.0f, -1.0f, -1.0f},{0.5f,0.5f,0.5f},{0.0f,-1.0f,0.0f},{1.0f,0.0f}},
	    {{-1.0f, -1.0f,  1.0f},{0.5f,0.5f,0.5f},{0.0f,-1.0f,0.0f},{0.0f,0.0f}},
	    {{ 1.0f, -1.0f,  1.0f},{0.5f,0.5f,0.5f},{0.0f,-1.0f,0.0f},{1.0f,1.0f}},
	};

	static inline const std::vector<Vertex> cubeVertices = {
		{{ -0.5f, -0.5f, -0.5f},{ 0.5f, 0.5f, 0.5f},{ 0.0f,  0.0f, -1.0f}, {.0, .0}},
		{{  0.5f, -0.5f, -0.5f},{ 0.5f, 0.5f, 0.5f},{ 0.0f,  0.0f, -1.0f}, {1., 0.}}, 
		{{  0.5f,  0.5f, -0.5f},{ 0.5f, 0.5f, 0.5f},{ 0.0f,  0.0f, -1.0f}, {1., 1.}}, 
		{{ -0.5f,  0.5f, -0.5f},{ 0.5f, 0.5f, 0.5f},{ 0.0f,  0.0f, -1.0f}, {0., 1.}}, 

		{{ -0.5f, -0.5f,  0.5f},{ 0.5f, 0.5f, 0.5f},{ 0.0f,  0.0f, 1.0f} , {.0, .0}},
		{{  0.5f, -0.5f,  0.5f},{ 0.5f, 0.5f, 0.5f},{ 0.0f,  0.0f, 1.0f} , {1., .0}},
		{{  0.5f,  0.5f,  0.5f},{ 0.5f, 0.5f, 0.5f},{ 0.0f,  0.0f, 1.0f} , {1., 1.}},
		{{ -0.5f,  0.5f,  0.5f},{ 0.5f, 0.5f, 0.5f},{ 0.0f,  0.0f, 1.0f} , {.0, 1.}},

		{{ -0.5f,  0.5f,  0.5f},{ 0.5f, 0.5f, 0.5f},{-1.0f,  0.0f,  0.0f}, {1., 0.}},
		{{ -0.5f,  0.5f, -0.5f},{ 0.5f, 0.5f, 0.5f},{-1.0f,  0.0f,  0.0f}, {1., 1.}},
		{{ -0.5f, -0.5f, -0.5f},{ 0.5f, 0.5f, 0.5f},{-1.0f,  0.0f,  0.0f}, {.0, 1.}},
		{{ -0.5f, -0.5f,  0.5f},{ 0.5f, 0.5f, 0.5f},{-1.0f,  0.0f,  0.0f}, {.0, .0}},

		{{  0.5f,  0.5f,  0.5f},{ 0.5f, 0.5f, 0.5f},{ 1.0f,  0.0f,  0.0f}, {1., .0}},
		{{  0.5f,  0.5f, -0.5f},{ 0.5f, 0.5f, 0.5f},{ 1.0f,  0.0f,  0.0f}, {1., 1.}},
		{{  0.5f, -0.5f, -0.5f},{ 0.5f, 0.5f, 0.5f},{ 1.0f,  0.0f,  0.0f}, {.0, 1.}},
		{{  0.5f, -0.5f,  0.5f},{ 0.5f, 0.5f, 0.5f},{ 1.0f,  0.0f,  0.0f}, {0., .0}},

		{{ -0.5f, -0.5f, -0.5f},{ 0.5f, 0.5f, 0.5f},{ 0.0f, -1.0f,  0.0f}, {0., 1.}},
		{{  0.5f, -0.5f, -0.5f},{ 0.5f, 0.5f, 0.5f},{ 0.0f, -1.0f,  0.0f}, {1., 1.}},
		{{  0.5f, -0.5f,  0.5f},{ 0.5f, 0.5f, 0.5f},{ 0.0f, -1.0f,  0.0f}, {1., 0.}},
		{{ -0.5f, -0.5f,  0.5f},{ 0.5f, 0.5f, 0.5f},{ 0.0f, -1.0f,  0.0f}, {0., 0.}},

		{{ -0.5f,  0.5f, -0.5f},{ 0.5f, 0.5f, 0.5f},{ 0.0f,  1.0f,  0.0f}, {.0, 1.}},
		{{  0.5f,  0.5f, -0.5f},{ 0.5f, 0.5f, 0.5f},{ 0.0f,  1.0f,  0.0f}, {1., 1.}},
		{{  0.5f,  0.5f,  0.5f},{ 0.5f, 0.5f, 0.5f},{ 0.0f,  1.0f,  0.0f}, {1., .0}},
		{{ -0.5f,  0.5f,  0.5f},{ 0.5f, 0.5f, 0.5f},{ 0.0f,  1.0f,  0.0f}, {0., 0.}},
	};


	static inline const std::vector<uint32_t> cubeIndices = {
	    // BACK (-Z) vertices 0–3
	    0, 1, 2,
	    2, 3, 0,

	    // FRONT (+Z) vertices 4–7
	    4, 5, 6,
	    6, 7, 4,

	    // LEFT (-X) vertices 8–11
	    8, 9,10,
	   10,11, 8,

	    // RIGHT (+X) vertices 12–15
	   12,13,14,
	   14,15,12,

	    // BOTTOM (-Y) vertices 16–19
	   16,17,18,
	   18,19,16,

	    // TOP (+Y) vertices 20–23
	   20,21,22,
	   22,23,20
	};

	static inline const std::vector<uint32_t> quadIndices = {
	    4, 5, 6,
	    6, 7, 4,
	};
};

#endif

#include <assimp/mesh.h>
#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include "./engine/engine.h"
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>
#include <chrono>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <iostream>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <algorithm>
#include <vector>
#include <cstdlib>
#include <cstdint>
#include <limits>
#include <optional>
#include <map>
#include <unordered_map>
#include <array>
#include <random>
#include "particle.h"

const std::string MODEL_PATH = "models/Sponza-master/sponza.obj";
size_t MESH_COUNT = 0;
const uint32_t OBJECT_COUNT = 10;
const uint32_t MAX_POINT_LIGHTS = 4;
const uint32_t PARTICLE_COUNT = 8192;
const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;
const float FAR_PLANE = 400.f;
//const std::string MODEL_PATH = "models/Viking Room/Viking Room.obj";
const std::string TEXTURE_PATH = "textures/container.png";
const std::string CUBEMAP_PATH = "textures/skybox/";
const std::string SPECULAR_PATH = "textures/container_specular.png";
//const std::string MODEL_TEXTURE_DIRECTORY = "models/Viking Room/";
const std::string MODEL_TEXTURE_DIRECTORY = "models/Sponza-master/";
uint32_t TEXTURE_COUNT = 0;
std::vector<std::string> texturePaths; 
const int MAX_FRAMES_IN_FLIGHT = 2;
uint32_t currentFrame = 0;
float lastFrameTime = 0.f;
float yaw = -90.f;
float pitch = 0.f;
float lastX = WIDTH/2;		
float lastY = HEIGHT/2;		
float steps = 0.;		
float verticalSteps = 0.;		
double lastTime = 0.f;
bool firstMouse = true;
glm::vec3 cameraPos = glm::vec3(0., 0., 3.);
glm::vec3 cameraFront = glm::vec3(0.f, 0.f, -1.f);
glm::vec3 cameraUp = glm::vec3(0.f,1.f, 0.f);

Camera camera(cameraPos, cameraFront, cameraUp);

// testing commits

std::ostream& operator<<(std::ostream& out, glm::vec3& vec)
{
	return out << "x: " << vec.x << " y: " << vec.y << " z: " << vec.z;
};

enum LightType
{
	DIRECTIONAL = 0,
	ATTENUATION = 1,
	SPOTLIGHT
};

std::vector<Vertex> modelVertices;
std::vector<uint32_t> indices;
std::vector<VkBuffer> vertexBuffers;
VkBuffer vertexCubeBuffer;
VkBuffer vertexCubemapBuffer;
std::vector<VkDeviceMemory> vertexBufferMemories;
VkDeviceMemory vertexCubeBufferMemory;
VkDeviceMemory vertexCubemapBufferMemory;

void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator)
{
	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");

	if (func != nullptr)
	{
		func(instance, debugMessenger, pAllocator);
	}
}

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
	if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS)
		steps += 0.1;
	if (glfwGetKey(window, GLFW_KEY_H) == GLFW_PRESS)
		steps -= 0.1;
	if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS)
		verticalSteps += 0.1;
	if (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS)
		verticalSteps -= 0.1;
}
// TODO: create cubemap vertex buffer and index buffer then you should be good
void mouse_callback(GLFWwindow * window, double xpos, double ypos);

class HelloTriangleApplication
{
public:
	void run()
	{
		Engine engine = Engine();
		engine.init();
	}

private:
	void cleanup()
	{
		cleanupSwapChain();

		delete model;

		vkDestroySampler(device, textureSampler, nullptr);
		vkDestroyImageView(device, textureImageView, nullptr);

		vkDestroyImage(device, textureImage, nullptr);
		vkFreeMemory(device, textureImageMemory, nullptr);

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
		{
			vkDestroyBuffer(device, uniformBuffers[i], nullptr);
			vkFreeMemory(device, uniformBuffersMemory[i], nullptr);
		}

		vkDestroyDescriptorPool(device, descriptorPool, nullptr);

		vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);

		vkDestroyBuffer(device, indexBuffer, nullptr);
		vkFreeMemory(device, indexBufferMemory, nullptr);
		
		vkDestroyBuffer(device, vertexCubeBuffer, nullptr);
		vkFreeMemory(device, vertexCubeBufferMemory, nullptr);
		
		for (size_t i = 0; i < vertexBuffers.size(); i++)
		{
			vkDestroyBuffer(device, vertexBuffers[i], nullptr);
			vkFreeMemory(device, vertexBufferMemories[i], nullptr);
		}

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
		{
			vkDestroySemaphore(device, imageAvailableSemaphores[i], nullptr);
			vkDestroySemaphore(device, renderFinishedSemaphores[i], nullptr);
			vkDestroyFence(device, inFlightFences[i], nullptr);
		}

		vkDestroyCommandPool(device, commandPool, nullptr);

		vkDestroyPipeline(device, graphicsPipeline, nullptr);
		vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
		vkDestroyRenderPass(device, renderPass, nullptr);

		vkDestroyDevice(device, nullptr);

		if (enableValidationLayers)
		{
			DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
		}

		vkDestroySurfaceKHR(instance, surface, nullptr);
		
		vkDestroyInstance(instance, nullptr);

		glfwDestroyWindow(window);

		glfwTerminate();
	}

	std::vector<const char*> getRequiredExtensions()
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

	bool checkValidationLayerSupport()
	{
		uint32_t layerCount;
		vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

		std::vector<VkLayerProperties> availableLayers(layerCount);
		vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

		for (const char* layerName : validationLayers)
		{
			bool layerFound = false;

			for (const auto& layerProperties : availableLayers)
			{
				if (strcmp(layerName, layerProperties.layerName) == 0)
				{
					layerFound = true;
					break;
				}
			}

			if (!layerFound)
			{
				return false;
			}

		}

		return true;

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

	static void framebufferResizeCallback(GLFWwindow* window, int width, int height)
	{

		auto app = reinterpret_cast<HelloTriangleApplication*>(glfwGetWindowUserPointer(window));
		app->framebufferResized = true;

	}

};

int main()
{
	HelloTriangleApplication app;

	try
	{
		app.run();
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;

}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	camera.move(xpos, ypos);
}


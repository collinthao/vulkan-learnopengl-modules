#include <string>
#include <assimp/mesh.h>
#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include "mesh.h"
#include <vector>
#include <map>
#include <iostream>
#include <string_view>
#include <thread>
#include "../config/vulkanConfig.h"
#include "../core/buffer/buffer.h"
#include <stb_image.h>

struct TextureInfo
{
	stbi_uc * pixels;
	int texWidth, texHeight, texChannels;
	bool mapped = false;
};

struct DescriptorInfo
{
	std::vector<std::vector<VkDescriptorSet>> sets;
};

struct Uniform
{
	std::vector<std::vector<void *>>         mapped;
	std::vector<std::vector<VkBuffer>>       buffer;
	std::vector<std::vector<VkDeviceMemory>> memory;
};

class Model
{
public:
	Model();
	Model(std::string path, const VkDevice& device, const VkPhysicalDevice& physicalDevice, const VkQueue& queue);
	VkDescriptorSetLayout layout;
	std::array<Uniform, 2> uniforms; 
	std::array<DescriptorInfo, 2> descriptors; 
	std::vector<Mesh> meshes;
	std::vector<Texture> textures_loaded;
	std::unordered_map<std::string, TextureInfo> textures_mapped;
	std::string directory;
	bool gammaCorrection;
	bool baked = false;

	uint32_t nodeIndex = 0;

	size_t indexSize = 0;
	size_t vertexSize = 0;

	#if defined(_WIN32) || defined(_WIN64)
		const std::string ROOT_DIR = std::string{GetExecutableDir() + '/'};
	#else
		const std::string ROOT_DIR = std::string{PROJECT_ROOT_DIR};
	#endif

	struct
	{
		std::vector<VkBuffer> buffers;
		std::vector<VkBuffer> index;
		std::vector<VkDeviceMemory> memory;
		std::vector<VkDeviceMemory> indexMemory;
	} buffer;

	struct
	{
		VkSampler sampler;
		std::vector<VkImage>     image;
		std::vector<VkImageView> imageView;
		std::vector<VkDeviceMemory> imageMemory;
	} texture;		

	void getMeshes();
	void processNode(aiNode *node, const aiScene *scene);
	Mesh processMesh(aiMesh*mesh, const aiScene * scene);

	template <typename UniformData>
	void bind(UniformData u, uint32_t frame, uint32_t index)
	{
		for (size_t i = 0; i < meshes.size(); i++)
		{
			memcpy(uniforms[frame].mapped[index][i], &u, sizeof(u));
		};
	};	

	template <typename UniformStruct>
	void setupUniforms()
	{
		VkDeviceSize objectBufferSize = sizeof(UniformStruct);	
		for (size_t j = 0; j < VulkanConfig::MAX_FRAMES_IN_FLIGHT; j++)
		{
			uniforms[j].buffer.resize(2);
			uniforms[j].mapped.resize(2);
			uniforms[j].memory.resize(2);

			// magic number for now
			for (size_t k = 0; k < 2; k++)
			{
				uniforms[j].buffer[k].resize(meshes.size());
				uniforms[j].mapped[k].resize(meshes.size());
				uniforms[j].memory[k].resize(meshes.size());
				for (size_t i = 0; i < meshes.size(); i++)
				{
					Buffer::create(objectBufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, uniforms[j].buffer[k][i], uniforms[j].memory[k][i], VulkanConfig::device, VulkanConfig::physicalDevice);
					
					vkMapMemory(VulkanConfig::device, uniforms[j].memory[k][i], 0, objectBufferSize, 0, &uniforms[j].mapped[k][i]);
				};
			};
		};
	};

	template <typename UniformStruct2>
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
	
		if (vkCreateDescriptorSetLayout(VulkanConfig::device, &layoutInfo, nullptr, &layout))
		{
			throw std::runtime_error("Failed to create descriptor set layout!");
		};	
		
		std::array<VkDescriptorPoolSize, 2> poolSizes{};
		poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		poolSizes[0].descriptorCount = static_cast<uint32_t>(this->meshes.size()* VulkanConfig::MAX_FRAMES_IN_FLIGHT) * 2 * 2;

		poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

		poolSizes[1].descriptorCount = static_cast<uint32_t>(this->meshes.size() * VulkanConfig::MAX_FRAMES_IN_FLIGHT) * 2 * 2;

		VkDescriptorPoolCreateInfo poolInfo
		{
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
			.maxSets = static_cast<uint32_t>(meshes.size() * VulkanConfig::MAX_FRAMES_IN_FLIGHT) * 2 * 2,
			.poolSizeCount = static_cast<uint32_t>(poolSizes.size()),
			.pPoolSizes = poolSizes.data()
		};	
		
		if (vkCreateDescriptorPool(VulkanConfig::device, &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create descriptor pool!");		
		}
			
		descriptors[0].sets.resize(2);
		descriptors[1].sets.resize(2);

		std::array<VkDescriptorSetLayout, 1> layouts{};
		layouts.fill(layout);	
	
		VkDescriptorSetAllocateInfo allocInfo
		{
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
			.descriptorPool = descriptorPool,
			.descriptorSetCount = static_cast<uint32_t>(layouts.size()),
			.pSetLayouts = layouts.data()
		};	  

		for (size_t j = 0; j < VulkanConfig::MAX_FRAMES_IN_FLIGHT; j++)
		{
			for (size_t k = 0; k < 2; k++)
			{
				descriptors[0].sets[k].resize(this->meshes.size());
				descriptors[1].sets[k].resize(this->meshes.size());
				for (size_t i = 0; i < this->meshes.size(); i++)
				{
				if (vkAllocateDescriptorSets(VulkanConfig::device, &allocInfo, &descriptors[j].sets[k][i]) != VK_SUCCESS)

					{
						throw std::runtime_error("failed to allocate descriptor sets!");
					};

					VkDescriptorBufferInfo bufferInfo{
						.buffer = uniforms[j].buffer[k][i],
						.offset = 0,
						.range = sizeof(UniformStruct2)
					};
					
					VkDescriptorImageInfo imageInfo
					{
						.sampler = texture.sampler,
						.imageView = texture.imageView[i],
						.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
					};
					
					std::array<VkWriteDescriptorSet, 2> descriptorWrites{};
					
					descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
					descriptorWrites[0].dstSet = descriptors[j].sets[k][i];
					descriptorWrites[0].dstBinding = 0;
					descriptorWrites[0].dstArrayElement = 0;
					descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
					descriptorWrites[0].descriptorCount = 1;
					descriptorWrites[0].pBufferInfo = &bufferInfo;
					
					descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
					descriptorWrites[1].dstSet = descriptors[j].sets[k][i];
					descriptorWrites[1].dstBinding = 1;
					descriptorWrites[1].dstArrayElement = 0;
					descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
					descriptorWrites[1].descriptorCount = 1;
					descriptorWrites[1].pImageInfo = &imageInfo;
					
					vkUpdateDescriptorSets(VulkanConfig::device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(),0, nullptr);	
				};
			};
		};

	};

private:

	std::string modelPath;
	VkDevice device;
	VkQueue queue;
	VkPhysicalDevice physicalDevice;
	std::vector<Texture> loadMaterialTextures(aiMaterial * mat, aiTextureType type, std::string typeName, const aiScene * scene);
	void setupModelData();
	void setupBuffers(const Mesh& mesh, const int& index);
	void setupIndexBuffers(const Mesh& mesh, const int& index);
	void setupImagesEmbedded(const int& index);
	void setupImages(const int& index);
	void setupImageViews(const int& index);
	void setupSampler();
	uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
};


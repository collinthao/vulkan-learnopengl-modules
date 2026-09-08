#include "model.h"
#include "../core/commandBuffer/commandBuffer.h"

unsigned int TextureFromFile(const char * path, const std::string & directory, bool gamma)
{
	std::string filename = std::string(path);
	filename = directory + '/' + filename;

	unsigned int textureID;

	return 0;	
}

Model::Model(){};


Model::Model(std::string path, const VkDevice& device, const VkPhysicalDevice& physicalDevice, const VkQueue& queue)
	:
      modelPath(this->ROOT_DIR + path),
      device(device),
      physicalDevice(physicalDevice), 
      queue(queue)
{
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(ROOT_DIR + path,
		aiProcess_CalcTangentSpace |
		aiProcess_Triangulate |
		aiProcess_JoinIdenticalVertices |
		aiProcess_SortByPType);

	if (nullptr == scene)
	{
		throw std::runtime_error("Failed to get path!" + ROOT_DIR + path);
	}

	processNode(scene->mRootNode, scene);
	setupModelData();	
}

void Model::setupSampler()
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
	
	if (vkCreateSampler(VulkanConfig::device, &samplerInfo, nullptr, &texture.sampler))
	{
		throw std::runtime_error("failed to create sampler!\n");	
	};
}

void Model::setupIndexBuffers(const Mesh& mesh, const int& index) 
{
		VkDeviceSize bufferSize = sizeof(mesh.indices[0]) * mesh.indices.size();				
		
		indexSize += bufferSize;

		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;
		
		Buffer::create(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory, device, physicalDevice);
		
		void * data;
		vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);	

		memcpy(data, mesh.indices.data(), (size_t)bufferSize);

		vkUnmapMemory(device, stagingBufferMemory);
		Buffer::create(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, buffer.index[index], buffer.indexMemory[index], device, physicalDevice);
		
		Buffer::copyBuffer(stagingBuffer, buffer.index[index], bufferSize);
		
		vkDestroyBuffer(device, stagingBuffer, nullptr);
		vkFreeMemory(device, stagingBufferMemory, nullptr);
};

void Model::setupBuffers(const Mesh& mesh, const int& index)
{
	VkDeviceSize bufferSize = sizeof(mesh.vertices[0]) * mesh.vertices.size();				
		
	vertexSize += bufferSize;	

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	
	Buffer::create(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory, device, physicalDevice);
	
	void * data;
	vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);	

	memcpy(data, mesh.vertices.data(), (size_t)bufferSize);
	vkUnmapMemory(device, stagingBufferMemory);
	Buffer::create(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, buffer.buffers[index], buffer.memory[index], device, physicalDevice);
	
	Buffer::copyBuffer(stagingBuffer, buffer.buffers[index], bufferSize);
	
	vkDestroyBuffer(device, stagingBuffer, nullptr);
	vkFreeMemory(device, stagingBufferMemory, nullptr);
};

void Model::setupImagesEmbedded(const int& index)
{
	stbi_uc* pixels;
	int texWidth, texHeight, texChannels;
	
	if (meshes[index].textures.empty())
	{
		Texture data = meshes[0].textures[0];
		if (textures_mapped[data.path].mapped)
		{
			pixels = textures_mapped[data.path].pixels;
			texWidth = textures_mapped[data.path].texWidth;
			texHeight = textures_mapped[data.path].texHeight;
			texChannels = textures_mapped[data.path].texChannels;
			int size = (size_t)(texWidth * texHeight * texChannels);
		}
		else 
		{
			pixels = stbi_load_from_memory(reinterpret_cast<unsigned char *>(data.data),data.width, &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);		
			
			textures_mapped[data.path] = {pixels, texWidth, texHeight, texChannels, true};
		};
	}	
	else
	{
		Texture data = meshes[index].textures[0];
		if (textures_mapped[data.path].mapped)
		{
			pixels = textures_mapped[data.path].pixels;
			texWidth = textures_mapped[data.path].texWidth;
			texHeight = textures_mapped[data.path].texHeight;
			texChannels = textures_mapped[data.path].texChannels;
			int size = (size_t)(texWidth * texHeight * texChannels);
		}
		else 
		{
			pixels = stbi_load_from_memory(reinterpret_cast<unsigned char*>(data.data), data.width, &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);		
			textures_mapped[data.path] = {pixels, texWidth, texHeight, texChannels, true};
		};
	};

	if (!pixels)
	{
		throw std::runtime_error("failed to load texture image dude!");	
	};
	
	VkDeviceSize imageSize = texWidth * texHeight * 4;
	int mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(texWidth, texHeight)))) + 1;	

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	
	Buffer::create(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT|VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory, device, physicalDevice);		
	
	void * data;
	vkMapMemory(device, stagingBufferMemory, 0, imageSize, 0, &data);
	
	memcpy(data, pixels, static_cast<size_t>(imageSize));	
	vkUnmapMemory(device, stagingBufferMemory);
	
	
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

	if(vkCreateImage(device, &imageInfo, nullptr, &texture.image[index]))
	{
		throw std::runtime_error("failed to create image!");
	};	
	
	VkMemoryRequirements memRequirements;
	vkGetImageMemoryRequirements(device, texture.image[index], &memRequirements);
	
	VkMemoryAllocateInfo allocInfo{
		.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
		.allocationSize = memRequirements.size,
		.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
	};
	
	if (vkAllocateMemory(device, &allocInfo, nullptr, &texture.imageMemory[index]) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to allocate image memory!");	
	};
	
	vkBindImageMemory(device, texture.image[index], texture.imageMemory[index], 0);
	
	VkCommandBuffer commandBuffer = CommandBuffer::beginSingleTimeCommands(device);		
	
	VkImageMemoryBarrier barrier{
		.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
		.srcAccessMask = 0,
		.dstAccessMask = 0,
		.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
		.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
		.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
		.image = texture.image[index]};

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
	
//	CommandBuffer::endSingleTimeCommands(commandBuffer, queue, device);		

//	commandBuffer = CommandBuffer::beginSingleTimeCommands(device);	
	
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
		texture.image[index],
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		1,
		&region);	

//	CommandBuffer::endSingleTimeCommands(commandBuffer, queue, device);

//	commandBuffer = CommandBuffer::beginSingleTimeCommands(device);	
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
	
	CommandBuffer::endSingleTimeCommands(commandBuffer, queue, device);		

	vkDestroyBuffer(device, stagingBuffer, nullptr);
	vkFreeMemory(device, stagingBufferMemory, nullptr);
};

void Model::setupImageViews(const int& index)
{
	VkImageViewCreateInfo viewInfo{
		.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
		.image = texture.image[index],
		.viewType = VK_IMAGE_VIEW_TYPE_2D,
		.format = VK_FORMAT_R8G8B8A8_SRGB};		

		viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = 1;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = 1;

	if (vkCreateImageView(device, &viewInfo, nullptr, &texture.imageView[index]))
	{
		throw std::runtime_error("failed to create image view!");	
	};
};

void Model::setupImages(const int& index)
{
	stbi_uc* pixels;
	int texWidth, texHeight, texChannels;
	
	if (meshes[index].textures.empty())
	{
		const std::string path = "/" + meshes[0].textures[0].path;
		if (textures_mapped[path].mapped)
		{
			pixels = textures_mapped[path].pixels;
			texWidth = textures_mapped[path].texWidth;
			texHeight = textures_mapped[path].texHeight;
			texChannels = textures_mapped[path].texChannels;
			int size = (size_t)(texWidth * texHeight * texChannels);
		}
		else 
		{
			pixels = stbi_load(path.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);		
			
			textures_mapped[path] = {pixels, texWidth, texHeight, texChannels, true};
		};
	}	
	else
	{
		const std::string path = ROOT_DIR + meshes[index].textures[0].path;
		if (textures_mapped[path].mapped)
		{
			pixels = textures_mapped[path].pixels;
			texWidth = textures_mapped[path].texWidth;
			texHeight = textures_mapped[path].texHeight;
			texChannels = textures_mapped[path].texChannels;
			int size = (size_t)(texWidth * texHeight * texChannels);
		}
		else 
		{
			pixels = stbi_load(path.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);		
			textures_mapped[path] = {pixels, texWidth, texHeight, texChannels, true};
		};
	};

	if (!pixels)
	{
		throw std::runtime_error("failed to load texture image dude!" + ROOT_DIR + meshes[0].textures[0].path);	
	};
	
	VkDeviceSize imageSize = texWidth * texHeight * 4;
	int mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(texWidth, texHeight)))) + 1;	

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	
	Buffer::create(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT|VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory, device, physicalDevice);		
	
	void * data;
	vkMapMemory(device, stagingBufferMemory, 0, imageSize, 0, &data);
	
	memcpy(data, pixels, static_cast<size_t>(imageSize));	
	vkUnmapMemory(device, stagingBufferMemory);
	
	
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

	if(vkCreateImage(device, &imageInfo, nullptr, &texture.image[index]))
	{
		throw std::runtime_error("failed to create image!");
	};	
	
	VkMemoryRequirements memRequirements;
	vkGetImageMemoryRequirements(device, texture.image[index], &memRequirements);
	
	VkMemoryAllocateInfo allocInfo{
		.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
		.allocationSize = memRequirements.size,
		.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
	};
	
	if (vkAllocateMemory(device, &allocInfo, nullptr, &texture.imageMemory[index]) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to allocate image memory!");	
	};
	
	vkBindImageMemory(device, texture.image[index], texture.imageMemory[index], 0);
	
	VkCommandBuffer commandBuffer = CommandBuffer::beginSingleTimeCommands(device);		
	
	VkImageMemoryBarrier barrier{
		.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
		.srcAccessMask = 0,
		.dstAccessMask = 0,
		.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
		.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
		.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
		.image = texture.image[index]};

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
	
//	CommandBuffer::endSingleTimeCommands(commandBuffer, queue, device);		

//	commandBuffer = CommandBuffer::beginSingleTimeCommands(device);	
	
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
		texture.image[index],
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		1,
		&region);	

//	CommandBuffer::endSingleTimeCommands(commandBuffer, queue, device);

//	commandBuffer = CommandBuffer::beginSingleTimeCommands(device);	
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
	
	CommandBuffer::endSingleTimeCommands(commandBuffer, queue, device);		

	vkDestroyBuffer(device, stagingBuffer, nullptr);
	vkFreeMemory(device, stagingBufferMemory, nullptr);
};

void Model::setupModelData()
{
	const size_t meshCount = meshes.size();

	buffer.index.resize(meshCount);
	buffer.indexMemory.resize(meshCount);
	buffer.buffers.resize(meshCount);	
	buffer.memory.resize(meshCount);	

	texture.imageView.resize(meshCount);		
	texture.imageMemory.resize(meshCount);		
	texture.image.resize(meshCount);		

	setupSampler();

	double overall = glfwGetTime();
	for (size_t i = 0; i < meshCount; i++)
	{			
		double time = glfwGetTime();
		const Mesh& mesh = meshes[i];
		setupIndexBuffers(mesh, i);
		setupBuffers(mesh, i);
		if (baked)
		{
			setupImagesEmbedded(i);
		}
		else
		{
			setupImages(i);
		};
		setupImageViews(i);
	};

	for (const auto& [key, value] : textures_mapped)
	{
		stbi_image_free(value.pixels);
	};

	std::cout << "Overall time: " << glfwGetTime() - overall << " seconds \n";
};

void Model::processNode(aiNode *node, const aiScene *scene)
{
	nodeIndex++;
	for (unsigned int i = 0; i < node->mNumMeshes; i++)
	{
		aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
		meshes.push_back(processMesh(mesh, scene));
	}

	for (unsigned int i = 0; i < node->mNumChildren; i++)
	{
		processNode(node->mChildren[i], scene);
	}
};

Mesh Model::processMesh(aiMesh*mesh, const aiScene * scene)
{
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;
	std::vector<Texture> textures;

	for (unsigned int i = 0; i < mesh->mNumVertices; i++)
	{
		Vertex vertex;
		glm::vec3 vector;
		vector.x = mesh->mVertices[i].x;
		vector.y = mesh->mVertices[i].y;
		vector.z = mesh->mVertices[i].z;
		vertex.pos = vector;
		
		vector.x = mesh->mNormals[i].x;
		vector.y = mesh->mNormals[i].y;
		vector.z = mesh->mNormals[i].z;
		vertex.normal = vector;

		if(mesh->mTextureCoords[0])
		{
			glm::vec2 vec;
			vec.x = mesh->mTextureCoords[0][i].x;
			vec.y = mesh->mTextureCoords[0][i].y;
			vertex.texCoord = vec;
		}
		else
			vertex.texCoord = glm::vec2(0.f, 0.f);

		vertex.color = glm::vec3(1.);

		vertices.push_back(vertex);
	}

	for (unsigned int i = 0; i < mesh->mNumFaces; i++)
	{
		aiFace face = mesh->mFaces[i];
		for (unsigned int j = 0; j < face.mNumIndices; j++)
		{
			indices.push_back(face.mIndices[j]);
		}
	}

	if (mesh->mMaterialIndex >= 0)
	{
		aiMaterial * material = scene->mMaterials[mesh->mMaterialIndex];
		std::vector<Texture> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse", scene);

		if (diffuseMaps.size() > 0)
		{
			textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
		}	

		std::vector<Texture> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular", scene);
	
		if (specularMaps.size() > 0)
		{
			textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
		}	
	}
	
	return Mesh(vertices, indices, textures);
};

std::vector<Texture> Model::loadMaterialTextures(aiMaterial * mat, aiTextureType type, std::string typeName, const aiScene * scene)
{
	std::vector<Texture> textures;
	for (unsigned int i = 0; i < mat->GetTextureCount(type); i++)
	{
		aiString str;
		
		mat->GetTexture(type, i, &str);
		
		const aiTexture * s_texture = scene->GetEmbeddedTexture(str.C_Str());

		if (s_texture)
		{
			baked = true;
			bool skip = false;
		
			for (unsigned int j = 0; j < textures_loaded.size(); j++)
			{
				if (std::strcmp(textures_loaded[j].path.data(), str.C_Str()) == 0)
				{
					textures.push_back(textures_loaded[j]);
					skip = true;
					break;
				}
			}
			if (!skip)
			{
				Texture texture;
				texture.id = str.C_Str();
				texture.type = typeName;
				texture.path = str.C_Str();
				texture.width = s_texture->mWidth;
				texture.height = s_texture->mHeight;
				texture.data = s_texture->pcData;
				textures.push_back(texture); 
				textures_loaded.push_back(texture);
			}

		}
		else
		{
			bool skip = false;
		
			for (unsigned int j = 0; j < textures_loaded.size(); j++)
			{
				if (std::strcmp(textures_loaded[j].path.data(), str.C_Str()) == 0)
				{
					textures.push_back(textures_loaded[j]);
					skip = true;
					break;
				}
			}
			if (!skip)
			{
				Texture texture;
				texture.id = str.C_Str();
				texture.type = typeName;
				texture.path = str.C_Str();
				textures.push_back(texture); 
				textures_loaded.push_back(texture);
			}

		};
	}

	return textures;
}

uint32_t Model::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
	VkPhysicalDeviceMemoryProperties memProperties;
	vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

	for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
	{
		if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
		{
			return i;
		}
	}

	throw std::runtime_error("failed to find a suitable memory type!");
}
